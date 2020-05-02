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
ReactiveGrasp::solve(int iterations = 1000, int blocks = 100, std::vector<double> alphas = {1.0})
{
  // Starting a clock to count algorithm run time
  Timer timer;

  // Use std::random_device to generate seed to Random engine
  uint seed = std::random_device{}();
  Random::seed(seed);

  // A map to track each alpha performance
  std::map<double, AlphaInfo> alphasMap;

  // Initialize alphas map
  for (int i = 0; i < alphas.size(); i++)
    alphasMap[alphas[i]] = {1.0/alphas.size(), 0.0, 0};

  Solution best;
  int optimalIteration = 0;

  for (int it = 1; it <= iterations; it++) {
    // Go full greedy at first iteration
    double alpha = it == 1 ? 0.0 : getRandomAlpha(alphasMap);

    Solution curr = buildGreedyRandomizedSolution(alpha);
    curr = rvnd(curr, {reinsert, swapZeroOne, swapOneOne});

    // Erase any route without requests
    for (auto r = curr.routes.begin(); r != curr.routes.end(); )
      r = (r->empty()) ? curr.routes.erase(r) : r + 1;

    if (it == 1 || (curr.feasible() && (curr.cost < best.cost || !best.feasible()))) {
      best = curr;

      if (optimalIteration == 0 && fabs(best.cost - inst->optimalSolutions[inst->name]) < 0.01)
        optimalIteration = it;
    }

    if (it % blocks == 0)
      updateProbabilities(alphasMap, best.cost);

    // Remember: first iteration is full greedy, so no need to update alpha info
    if (it != 1) {
      int penalty = !curr.feasible() ? 1000 : 0; // Penalize alphas that generated infeasible solutions

      alphasMap[alpha].count++;
      alphasMap[alpha].cumulativeCost += curr.cost + penalty * curr.routes.size();
    }

    // for (std::pair<double, AlphaInfo> pair : alphasMap)
    //   printf("\nAvg %.2f = %.2f (%.2f)", pair.first, pair.second.avg(), pair.second.probability);

    Display::printProgress(best, (double) it/iterations);
  }

  double elapsedTime = timer.elapsedInMinutes();

  Display::printSolutionInfoWithElapsedTime(best, elapsedTime);

  for (std::pair<double, AlphaInfo> pair : alphasMap)
    printf("%.2f - %d times (%.2f%%)\n", pair.first, pair.second.count, pair.second.probability);

  printf("%d\n", optimalIteration);

  return std::make_tuple(best, elapsedTime, seed, optimalIteration);
}

double ReactiveGrasp::getRandomAlpha(std::map<double, AlphaInfo> alphasMap)
{
  double rand = Random::get(0.0, 1.0);
  double sum  = 0.0;

  for (std::pair<double, AlphaInfo> pair : alphasMap) {
    sum += pair.second.probability;

    if (rand <= sum)
      return pair.first;
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

      if (r.feasible() && r.cost < best.cost)
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
    auto move = Random::get(rvndMoves); // Get an iterator to a random move
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
          // Reset the search with updated route
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
        r1.feasible();

        for (int k2 = 0; k2 < s.routes.size(); k2++) {
          if (k1 != k2) {
            Route r2 = getCheapestFeasibleInsertion(req, s.routes[k2]);

            if (r1.cost + r2.cost < s.routes[k1].cost + s.routes[k2].cost) {
              // Reset the search with updated route
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

void ReactiveGrasp::updateProbabilities(std::map<double, AlphaInfo> &alphasMap, double bestCost)
{
  double qsum = 0.0;

  for (std::pair<double, AlphaInfo> pair : alphasMap)
    if (pair.second.avg() > 0)
      qsum += bestCost/pair.second.avg();

  for (std::pair<double, AlphaInfo> pair : alphasMap)
    alphasMap[pair.first].probability = (bestCost/pair.second.avg())/qsum;
}
