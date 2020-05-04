/**
 * @file   ReactiveGrasp.cpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "data-structures/Singleton.hpp"
#include "algorithms/ReactiveGrasp.hpp"
#include "utils/Timer.hpp"
#include "utils/Display.hpp"

Run ReactiveGrasp::solve(int iterations = 1000, int blocks = 100, std::vector<double> alphas = {0.5, 1.0})
{
  // Starting a clock to count algorithm run time
  Timer timer;

  // Use std::random_device to generate seed to Random engine
  unsigned int seed = std::random_device{}();
  Random::seed(seed);

  // A map to track each alpha performance
  std::map<double, AlphaInfo> alphasMap;

  // Initialize alphas map
  for (int i = 0; i < alphas.size(); i++)
    alphasMap[alphas[i]] = {1.0/alphas.size(), 0.0, 0};

  Solution best;
  int optimalIteration = -1;

  for (int it = 0; it <= iterations; it++) {
    // Go full greedy in first iteration
    double alpha = it == 0 ? 0.0 : getRandomAlpha(alphasMap);

    Solution curr = buildGreedyRandomizedSolution(alpha);
    curr = rvnd(curr, {reinsert, swapZeroOne, swapOneOne});

    // Erase any route without requests
    for (auto r = curr.routes.begin(); r != curr.routes.end(); )
      r = (r->empty()) ? curr.routes.erase(r) : r + 1;

    if (it == 0 || (curr.feasible() && (curr.cost < best.cost || !best.feasible()))) {
      best = curr;

      if (optimalIteration == -1 && fabs(best.cost - inst->optimalSolutions[inst->name]) < 0.01)
        optimalIteration = it;
    }

    // Remember: first iteration is full greedy, so no need to update alpha info
    if (it != 0) {
      if (it % blocks == 0)
        updateProbabilities(alphasMap, best.cost);

      // Penalize alphas that generated infeasible solutions
      int penalty = !curr.feasible() ? 1000 : 0;

      alphasMap[alpha].count++;
      alphasMap[alpha].cumulativeCost += curr.cost + penalty * curr.routes.size();
    }

    // for (std::pair<double, AlphaInfo> pair : alphasMap)
    //   printf("\nAvg %.2f = %.2f (%.2f)", pair.first, pair.second.avg(), pair.second.probability);

    Display::printProgress(best, (double) it/iterations);
  }

  // for (std::pair<double, AlphaInfo> pair : alphasMap)
  //   printf("%.2f - %d times (%.2f%%)\n", pair.first, pair.second.count, pair.second.probability);

  return Run(best, timer.elapsedMinutes(), seed, optimalIteration);
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
      solution.setRoute(candidate->route.vehicle, candidate->route);
    }

    // Save route to optimize the update of candidates
    Route selected = candidate->route;

    candidates.erase(candidate);

    // Update candidates
    for (Candidate &c : candidates)
      if (c.route == selected)
        c = {getCheapestFeasibleInsertion(c.request, solution), c.request};
  }

  solution.updateCost();

  return solution;
}

Route ReactiveGrasp::getCheapestFeasibleInsertion(Request req, Solution s)
{
  Route best;

  for (int k = 0; k < s.routes.size(); k++) {
    Route curr = getCheapestFeasibleInsertion(req, s.routes[k]);

    if (k == 0 || curr.cost < best.cost)
      best = curr;
  }

  return best;
}

Route ReactiveGrasp::getCheapestFeasibleInsertion(Request req, Route r)
{
  // Best insertion starts with infinity cost, we will update it during the search
  Route best = r;
  best.cost  = MAXFLOAT;

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
  START:

  for (Route &r : s.routes) {
    for (Node *node : r.path) {
      if (node->isPickup()) {
        Request req = inst->getRequest(node);
        Route  curr = r;

        // Erase request by value
        curr.path.erase(std::remove(curr.path.begin(), curr.path.end(), req.pickup),   curr.path.end());
        curr.path.erase(std::remove(curr.path.begin(), curr.path.end(), req.delivery), curr.path.end());

        // Reinsert the request
        curr = getCheapestFeasibleInsertion(req, r);

        if (curr.cost < r.cost) {
          // Update solution and reset the search
          r = curr;
          goto START;
        }
      }
    }
  }

  s.updateCost();

  return s;
}

Solution ReactiveGrasp::swapZeroOne(Solution s)
{
  START:

  for (Route &r1 : s.routes) {
    for (Node *node : r1.path) {
      if (node->isPickup()) {
        Request req   = inst->getRequest(node);
        Route   curr1 = r1;

        // Erase request by value
        curr1.path.erase(std::remove(curr1.path.begin(), curr1.path.end(), req.pickup),   curr1.path.end());
        curr1.path.erase(std::remove(curr1.path.begin(), curr1.path.end(), req.delivery), curr1.path.end());

       /* Route will be definitely feasible because removing a request from a feasible route does not
        * make it infeasible, but we call this procedure to update its member variables
        */
        curr1.feasible();

        for (Route &r2 : s.routes) {
          if (r1 != r2) {
            Route curr2 = getCheapestFeasibleInsertion(req, r2);

            if (curr1.cost + curr2.cost < r1.cost + r2.cost) {
              // Update solution and reset the search
              r1 = curr1;
              r2 = curr2;
              goto START;
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
  START:

  std::vector<Route> possibleRoutes;

  // Only routes with at least one request accommodated are eligible
  for (Route r : s.routes)
    if (!r.empty())
      possibleRoutes.push_back(r);

  // To perform the move, there must be at least two routes with requests to be swapped
  if (possibleRoutes.size() >= 2) {
    // Select two distinct random routes
    Route r1 = *Random::get(possibleRoutes);
    Route r2 = *Random::get(possibleRoutes);

    while (r2 == r1)
      r2 = *Random::get(possibleRoutes);

    // Select a random request in each route
    Request req1 = inst->getRequest(*Random::get(r1.path.begin() + 1, r1.path.end() - 1));
    Request req2 = inst->getRequest(*Random::get(r2.path.begin() + 1, r2.path.end() - 1));

    // Remove (by value) req1 from r1 and req2 from r2
    r1.path.erase(std::remove(r1.path.begin(), r1.path.end(), req1.pickup),   r1.path.end());
    r1.path.erase(std::remove(r1.path.begin(), r1.path.end(), req1.delivery), r1.path.end());

    r2.path.erase(std::remove(r2.path.begin(), r2.path.end(), req2.pickup),   r2.path.end());
    r2.path.erase(std::remove(r2.path.begin(), r2.path.end(), req2.delivery), r2.path.end());

    // Insert req2 in r1 and req1 in r2
    r1 = getCheapestFeasibleInsertion(req2, r1);
    r2 = getCheapestFeasibleInsertion(req1, r2);

    if (r1.cost + r2.cost < s.getRoute(r1.vehicle).cost + s.getRoute(r2.vehicle).cost) {
      // Update solution and reset the search
      s.setRoute(r1.vehicle, r1);
      s.setRoute(r2.vehicle, r2);
      goto START;
    }
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
