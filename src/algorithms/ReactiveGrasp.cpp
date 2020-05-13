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

  // Moves to be used in RVND
  std::vector<Move> moves = {reinsert, swapZeroOne, swapOneOne, removeStation};

  Solution best;
  int optimalIteration = -1;

  for (int it = 0; it <= iterations; it++) {
    // Reserve a first iteration to go full greedy
    double alpha = it == 0 ? 0.0 : getRandomAlpha(alphasMap);

    Solution curr = buildGreedyRandomizedSolution(alpha);
    curr = rvnd(curr, moves);

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
      int penalty = !curr.feasible() ? 1000 : 0; // Penalize alphas that generated infeasible solutions

      alphasMap[alpha].count++;
      alphasMap[alpha].sum += curr.cost + penalty * curr.routes.size();

      if (it % blocks == 0)
        updateProbabilities(alphasMap, best.cost);
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

  for (int p = 1; p < r.path.size(); p++) {
    r.path.insert(r.path.begin() + p, req.pickup);

    int lastEmptyPos = 1, load = 0, stationPos1 = -1;

    for (int i = 1; i < p; i++) {
      if (r.path[i]->isPickup())
        load++;
      else if (r.path[i]->isDelivery())
        load--;

      if (load == 0)
        lastEmptyPos = i + 1;
    }

    if (r.getStateOfCharge(p) < 0.0) {
      Node *station = inst->getNearestStation(r.path[lastEmptyPos - 1], r.path[lastEmptyPos]);
      r.path.insert(r.path.begin() + lastEmptyPos, station);
      stationPos1 = lastEmptyPos;
    }

    for (int d = p + 1; d < r.path.size(); d++) {
      r.path.insert(r.path.begin() + d, req.delivery);

      int stationPos2 = -1;

      if (r.getStateOfCharge(r.path.size() - 1) < r.vehicle.finalMinStateOfCharge) {
        Node *station = inst->getNearestStation(r.path[r.path.size() - 2], r.path[r.path.size() - 1]);
        r.path.insert(r.path.begin() + r.path.size() - 1, station);
        stationPos2 = r.path.size() - 2;
      }

      if (r.feasible() && r.cost < best.cost)
        best = r;

      if (stationPos2 != -1)
        r.path.erase(r.path.begin() + stationPos2);

      r.path.erase(r.path.begin() + d);
    }

    if (stationPos1 != -1)
      r.path.erase(r.path.begin() + stationPos1);

    r.path.erase(r.path.begin() + p);
  }

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
  Solution best = s;

  for (Route r : s.routes) {
    for (Node *node : r.path) {
      if (node->isPickup()) {
        Request req = inst->getRequest(node);
        Route curr = r;

        // Erase request by value
        curr.path.erase(std::remove(curr.path.begin(), curr.path.end(), req.pickup),   curr.path.end());
        curr.path.erase(std::remove(curr.path.begin(), curr.path.end(), req.delivery), curr.path.end());

        // Reinsert the request
        curr = getCheapestFeasibleInsertion(req, r);

        // Generate neighbor solution
        Solution neighbor = s;
        neighbor.setRoute(curr.vehicle, curr);
        neighbor.updateCost();

        if (neighbor.cost < best.cost)
          best = neighbor;
      }
    }
  }

  return best;
}

Solution ReactiveGrasp::swapZeroOne(Solution s)
{
  Solution best = s;

  for (Route r1 : s.routes) {
    for (Node *node : r1.path) {
      if (node->isPickup()) {
        Request req   = inst->getRequest(node);
        Route   curr1 = r1;

        // Erase request by value
        curr1.path.erase(std::remove(curr1.path.begin(), curr1.path.end(), req.pickup),   curr1.path.end());
        curr1.path.erase(std::remove(curr1.path.begin(), curr1.path.end(), req.delivery), curr1.path.end());

       /* Route will be definitely feasible because removing a request from a feasible route does not
        * make it infeasible, but we call this procedure to update costs
        */
        curr1.feasible();

        for (Route r2 : s.routes) {
          if (r1 != r2) {
            // Insert req in the new route
            Route curr2 = getCheapestFeasibleInsertion(req, r2);

            // Generate neighbor solution;
            Solution neighbor = s;
            neighbor.setRoute(curr1.vehicle, curr1);
            neighbor.setRoute(curr2.vehicle, curr2);
            neighbor.updateCost();

            if (neighbor.cost < best.cost)
              best = neighbor;
          }
        }
      }
    }
  }

  return best;
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
    Node *n1 = *Random::get(r1.path.begin() + 1, r1.path.end() - 1);
    Node *n2 = *Random::get(r2.path.begin() + 1, r2.path.end() - 1);

    while (n1->isStation())
      n1 = *Random::get(r1.path.begin() + 1, r1.path.end() - 1);

    while (n2->isStation())
      n2 = *Random::get(r2.path.begin() + 1, r2.path.end() - 1);

    Request req1 = inst->getRequest(n1);
    Request req2 = inst->getRequest(n2);

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

Solution ReactiveGrasp::removeStation(Solution s)
{
  Solution best = s;

  for (Route r : s.routes) {
    for (auto nodeIt = r.path.begin(); nodeIt != r.path.end(); nodeIt++) {
      if ((*nodeIt)->isStation()) {
        Node *predecessor = *(nodeIt - 1);
        Node *successor   = *(nodeIt + 1);

        int index = nodeIt - r.path.begin();

        double preToSuc = r.batteryLevels[index - 1] - r.vehicle.dischargingRate * inst->getTravelTime(predecessor, successor);

        // We have enough battery to go from predecessor to sucessor
        if (!successor->isDepot() && preToSuc > 0.0) {
          Route curr = r;
          curr.path.erase(curr.path.begin() + index);

          if (curr.feasible()) {
            // Generate neighbor solution
            Solution neighbor = s;
            neighbor.setRoute(curr.vehicle, curr);
            neighbor.updateCost();

            if (neighbor.cost < best.cost)
              best = neighbor;
          }
        }
      }
    }
  }

  return best;
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
