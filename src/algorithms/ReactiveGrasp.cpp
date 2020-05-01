/**
 * @file   ReactiveGrasp.cpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "data-structures/Singleton.hpp"
#include "algorithms/ReactiveGrasp.hpp"
#include "utils/Timer.hpp"
#include "utils/Display.hpp"

std::tuple<Solution, double, uint, int>
ReactiveGrasp::solve(int iterations = 100, int blocks = 10, std::vector<double> alphas = {1.0})
{
  // Starting a clock to count algorithm's run time
  Timer timer;

  // Use std::random_device to generate seed to Random engine
  uint seed = std::random_device{}();
  Random::seed(seed);

  std::vector<RandomFactor> randomFactors (alphas.size());

  // Initializing each alpha as random factor
  for (int i = 0; i < alphas.size(); i++) {
    RandomFactor rf;

    rf.alpha          = alphas[i];
    rf.probability    = 1.0/alphas.size();
    rf.q              = 0.0;
    rf.cumulativeCost = 0.0;
    rf.count          = 0;

    randomFactors[i] = rf;
  }

  Solution best;
  int optimalIteration = 0;

  for (int it = 1; it <= iterations; it++) {
    int index;
    double alpha;

    // Go full greedy at first iteration
    if (it == 1) {
      alpha = 0.0;
    }
    else {
      index = chooseRandomFactorIndex(randomFactors);
      alpha = randomFactors[index].alpha;
      randomFactors[index].count++;
    }

    Solution curr = buildGreedyRandomizedSolution(alpha);
    curr = rvnd(curr, {reinsert, swapZeroOne, swapOneOne});

    // Erase any route without requests
    for (auto k = curr.routes.begin(); k != curr.routes.end(); )
      k = (k->path.size() <= 2) ? curr.routes.erase(k) : k + 1;

    if (it == 1 || (curr.isFeasible() && curr.cost < best.cost)) {
      best = curr;

      if (optimalIteration == 0 && fabs(best.cost - inst->optimalSolutions[inst->name]) < 0.01)
        optimalIteration = it;
    }

    if (it % blocks == 0) {
      for (int i = 0; i < randomFactors.size(); i++) {
        double avg = randomFactors[i].count > 0 ? randomFactors[i].cumulativeCost/randomFactors[i].count : 0;

        if (avg > 0)
          randomFactors[i].q = best.cost/avg;
      }

      updateProbabilities(randomFactors);
    }

    if (it != 1) {
      int param = curr.routes.size() > inst->vehicles.size() ? 1000 : 0;
      randomFactors[index].cumulativeCost += curr.cost + param * curr.routes.size();
    }

    // printf("\n");
    // for (int i = 0; i < randomFactors.size(); i++) {
    //   double avg = randomFactors[i].count > 0 ? randomFactors[i].cumulativeCost/randomFactors[i].count : 0;
    //   printf("Avg %.2f = %.2f (%.2f)\n", randomFactors[i].alpha, avg, randomFactors[i].probability);
    // }

    Display::printProgress(best, (double) it/iterations);
  }

  double elapsedTime = timer.elapsedInMinutes();

  Display::printSolutionInfoWithElapsedTime(best, elapsedTime);

  for (int i = 0; i < randomFactors.size(); i++) {
    RandomFactor rf = randomFactors[i];
    printf("%2d. %.2f - %d times (%.2f%%)\n", i + 1, rf.alpha, rf.count, rf.probability);
  }

  printf("%d\n", optimalIteration);

  return std::make_tuple(best, elapsedTime, seed, optimalIteration);
}

int ReactiveGrasp::chooseRandomFactorIndex(std::vector<RandomFactor> randomFactors)
{
  double rand = Random::get(0.0, 1.0);
  double sum  = 0.0;

  for (int i = 0; i < randomFactors.size(); i++) {
    sum += randomFactors[i].probability;

    if (rand <= sum)
      return i;
  }

  return 0;
}

Solution ReactiveGrasp::buildGreedyRandomizedSolution(double alpha)
{
  Solution solution;

  for (Vehicle &v : inst->vehicles)
    solution.routes.push_back(Route(v));

  for (Route &route : solution.routes) {
    route.path.push_back(inst->getOriginDepot());
    route.path.push_back(inst->getDestinationDepot());
  }

  struct Candidate
  {
    Route   route;
    Request request;
  };

  std::vector<Candidate> candidates;

  // Initialize candidates
  for (Request req : inst->requests)
    candidates.push_back({getCheapestFeasibleInsertion(req, solution), req});

  while (!candidates.empty()) {
    std::sort(candidates.begin(), candidates.end(), [] (Candidate &c1, Candidate &c2) {
      return c1.route.cost < c2.route.cost;
    });

    // Get an iterator to a random candidate in restricted range
    auto candidate = Random::get(candidates.begin(), candidates.begin() + (int) (alpha * candidates.size()));

    if (candidate->route.cost == MAXFLOAT) {
     /* Request could not be feasibly inserted, so we open a new route (thus solution will be infeasible).
      * New vehicle is created by copying first vehicle of solution and changing its id.
      */
      Vehicle v = solution.routes[0].vehicle;
      v.id = solution.routes.size() + 1;

      solution.routes.push_back(Route(v, candidate->request));
    }
    else {
      solution.routes.at(candidate->route.vehicle.id - 1) = candidate->route;
    }

    // Save route to optimize the update of candidates
    Route selected = candidate->route;

    candidates.erase(candidate);

    // Update candidates
    for (int i = 0; i < candidates.size(); i++)
      if (candidates[i].route == selected)
        candidates[i] = {getCheapestFeasibleInsertion(candidates[i].request, solution), candidates[i].request};
  }

  solution.updateCost();

  return solution;
}

Route ReactiveGrasp::getCheapestFeasibleInsertion(Request req, Solution s)
{
  Route best;

  for (int k = 0; k < s.routes.size(); k++) {
    Route r = getCheapestFeasibleInsertion(req, s.routes[k]);

    if (k == 0 || r.cost < best.cost)
      best = r;
  }

  return best;
}

Route ReactiveGrasp::getCheapestFeasibleInsertion(Request req, Route r)
{
  // Best insertion starts with infinity cost, we will update it during the search
  Route best = r;
  best.cost = MAXFLOAT;

  // printf("Insert req (%d, %d) in route [", req.pickup->id, req.delivery->id);

  // for (Node *n : r.path)
  //   printf(" %d ", n->id);

  // printf("]\n");

  for (int p = 1; p < r.path.size(); p++) {
    r.path.insert(r.path.begin() + p, req.pickup);

    for (int d = p + 1; d < r.path.size(); d++) {
      r.path.insert(r.path.begin() + d, req.delivery);

      // double bat = 14.85;
      // int stationPos = -1;

      // for (int i = 1; i < r.path.size(); i++) {
      //   bat -= r.vehicle.dischargingRate * inst->getTravelTime(r.path[i - 1], r.path[i]);

      //   if (bat < 0) {
      //     Node *station;

      //     if (r.path[i - 1]->isDepot() || r.path[i]->isDepot())
      //       station = inst->getNearestStation(r.path[i - 1], r.path[i - 1]);
      //     else
      //       station = inst->getNearestStation(r.path[i - 1], r.path[i]);

      //     stationPos = i;
      //     r.path.insert(r.path.begin() + i, station);
      //     break;
      //   }
      // }

      // printf("\n\t=> [");
      // for (Node *n : r.path)
      //   printf(" %d ", n->id);
      // printf("]\n");

      if (r.isFeasible() && r.cost < best.cost)
        best = r;

      // for (int i = 0; i < r.path.size(); i++) {
      //   printf("A[%02d] = %6.4g\t", i,  r.arrivalTimes[i]);
      //   printf("B[%02d] = %6.4g\t", i,  r.serviceBeginningTimes[i]);
      //   printf("D[%02d] = %6.4g\t", i,  r.departureTimes[i]);
      //   printf("W[%02d] = %.4g\t",  i,  r.waitingTimes[i]);
      //   printf("R[%02d] = %.4g\t",  i,  r.rideTimes[i]);
      //   printf("Z[%02d] = %.4g\t",  i,  r.batteryLevels[i]);
      //   printf("C[%02d] = %.4g\t",  i,  r.chargingTimes[i]);
      //   printf("Q[%02d] = %d",      i,  r.load[i]);
      //   printf("\n");
      // }

      // if (stationPos != -1)
      //   r.path.erase(r.path.begin() + stationPos);

      r.path.erase(r.path.begin() + d);
    }

    r.path.erase(r.path.begin() + p);
  }

  // printf("\n");

  return best;
}

Solution ReactiveGrasp::rvnd(Solution s, std::vector<Move> moves)
{
  std::vector<Move> rvndMoves = moves;

  while (!rvndMoves.empty()) {
    // Get an iterator to a random move
    auto move = Random::get(rvndMoves);

    Solution neighbor = (*move)(s);

    if (neighbor.cost < s.cost) {
      s = neighbor;
      rvndMoves = moves;
    }
    else {
      rvndMoves.erase(move);
    }
  }

  return s;
}

Solution ReactiveGrasp::reinsert(Solution s)
{
  for (int k = 0; k < s.routes.size(); k++) {
    for (int i = 1; i < s.routes[k].path.size() - 1; i++) {
      Node *node = s.routes[k].path[i];

      if (node->isPickup()) {
        // Remove request from the route
        Request req = inst->getRequest(node);
        Route   r   = s.routes[k];

        r.path.erase(std::remove(r.path.begin(), r.path.end(), req.pickup),   r.path.end());
        r.path.erase(std::remove(r.path.begin(), r.path.end(), req.delivery), r.path.end());

        // Reinsert the request
        r = getCheapestFeasibleInsertion(req, r);

        if (r.cost < s.routes[k].cost) {
          s.routes[k] = r;
          i = 1;
        }
      }
    }
  }

  s.updateCost();

  return s;
}

Solution ReactiveGrasp::swapZeroOne(Solution s)
{
  for (int k1 = 0; k1 < s.routes.size(); k1++) {
    for (int i = 1; i < s.routes[k1].path.size() - 1; i++) {
      Node *node = s.routes[k1].path[i];

      if (node->isPickup()) {
        Request req = inst->getRequest(node);
        Route   r1  = s.routes[k1];

        r1.path.erase(std::remove(r1.path.begin(), r1.path.end(), req.pickup),   r1.path.end());
        r1.path.erase(std::remove(r1.path.begin(), r1.path.end(), req.delivery), r1.path.end());

        // Route will be definitely feasible, but we call this procedure to update its member variables
        r1.isFeasible();

        for (int k2 = 0; k2 < s.routes.size(); k2++) {
          if (k1 != k2) {
            Route r2 = getCheapestFeasibleInsertion(req, s.routes[k2]);

            if (r1.cost + r2.cost < s.routes[k1].cost + s.routes[k2].cost) {
              s.routes[k1] = r1;
              s.routes[k2] = r2;
              k1 = 0;
              i  = 1;
              break;
            }
          }
        }
      }
    }
  }

  s.updateCost();

  return s;
}

Solution ReactiveGrasp::swapOneOne(Solution s)
{
  std::vector<int> possibleRoutes;

  // Only routes with at least one request are eligible
  for (int k = 0; k < s.routes.size(); k++)
    if (s.routes[k].path.size() > 2)
      possibleRoutes.push_back(k);

  // To perform the move, there must be at least two routes with requests to be swapped
  if (possibleRoutes.size() >= 2) {
    bool improved;

    do {
      improved = false;

      // Select two distinct random routes
      int k1 = *Random::get(possibleRoutes);
      int k2 = *Random::get(possibleRoutes);

      while (k2 == k1)
        k2 = *Random::get(possibleRoutes);

      Route r1 = s.routes[k1];
      Route r2 = s.routes[k2];

      // Select two random requests in each route
      Node *node1 = *Random::get(r1.path.begin() + 1, r1.path.end() - 1);
      Node *node2 = *Random::get(r2.path.begin() + 1, r2.path.end() - 1);

      Request req1 = inst->getRequest(node1);
      Request req2 = inst->getRequest(node2);

      // Remove req1 from r1 and req2 from r2
      r1.path.erase(std::remove(r1.path.begin(), r1.path.end(), req1.pickup),   r1.path.end());
      r1.path.erase(std::remove(r1.path.begin(), r1.path.end(), req1.delivery), r1.path.end());

      r2.path.erase(std::remove(r2.path.begin(), r2.path.end(), req2.pickup),   r2.path.end());
      r2.path.erase(std::remove(r2.path.begin(), r2.path.end(), req2.delivery), r2.path.end());

      // Insert req2 in r1 and req1 in r2
      r1 = getCheapestFeasibleInsertion(req2, r1);
      r2 = getCheapestFeasibleInsertion(req1, r2);

      if (r1.cost + r2.cost < s.routes[k1].cost + s.routes[k2].cost) {
        s.routes[k1] = r1;
        s.routes[k2] = r2;
        improved = true;
      }
    }
    while (improved);
  }

  s.updateCost();

  return s;
}

void ReactiveGrasp::updateProbabilities(std::vector<RandomFactor> &randomFactors)
{
  double qsum = 0.0;

  for (RandomFactor rf : randomFactors)
    qsum += rf.q;

  for (int i = 0; i < randomFactors.size(); i++)
    randomFactors[i].probability = randomFactors[i].q/qsum;
}
