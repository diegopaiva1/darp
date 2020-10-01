/**
 * @file   ReactiveGrasp.cpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "data-structures/Singleton.hpp"
#include "algorithms/ReactiveGrasp.hpp"
#include "utils/Timer.hpp"
#include "utils/Display.hpp"

#include <iostream>

Run ReactiveGrasp::solve(int iterations, int blocks, std::vector<double> alphas)
{
  Timer timer; // Starting a clock to count algorithm's run time
  Solution best;
  double bestObj = MAXFLOAT;
  Run run;

  // Use std::random_device to generate seed to Random engine
  unsigned int seed = std::random_device{}();
  Random::seed(seed);

  // A map to track each alpha performance
  std::map<double, AlphaInfo> alphasMap;

  // Initialize alphas map
  for (int i = 0; i < alphas.size(); i++)
    alphasMap[alphas[i]] = {1.0/alphas.size(), 0.0, 0};

  // Moves to be used in RVND
  std::vector<Move> moves = {swapZeroOne, swapOneOne, reinsert};

  int noImprovementIt = 0;

  for (int it = 0; it <= iterations; it++) {
    // Reserve a first iteration to go full greedy
    double alpha = it == 0 ? 0.0 : getRandomAlpha(alphasMap);

    Solution init = buildGreedyRandomizedSolution(alpha);
    Solution curr = rvnd(init, moves);

    allocateStations(curr);

    double currObj = curr.objFuncValue();

    if (it == 0 || (curr.feasible() && (currObj < bestObj || !best.feasible()))) {
      best = curr;
      bestObj = currObj;
      run.bestAlpha = alpha;
      run.bestInit = init;
      run.bestIteration = it;
      noImprovementIt = 0;
    }
    else {
      noImprovementIt++;
    }

    // if (noImprovementIt == 500)
    //   break;

    // Remember: first iteration is full greedy, so no need to update alpha info
    if (it != 0) {
      int penalty = !curr.feasible() ? 10 : 1; // Penalize alphas that generated infeasible solutions

      alphasMap[alpha].count++;
      alphasMap[alpha].sum += currObj * penalty;

      if (it % blocks == 0)
        updateProbabilities(alphasMap, bestObj);
    }

    Display::printProgress(best.feasible(), bestObj, (double) it/iterations);
  }

  // Erase any route without requests from best
  for (auto r = best.routes.begin(); r != best.routes.end(); )
    r = (r->empty()) ? best.routes.erase(r) : r + 1;

  for (auto [alpha, info] : alphasMap)
    run.probDistribution[alpha] = info.probability;

  run.best = best;
  run.seed = seed;
  run.elapsedMinutes = timer.elapsedMinutes();

  return run;
}

double ReactiveGrasp::getRandomAlpha(std::map<double, AlphaInfo> alphasMap)
{
  double rand = Random::get(0.0, 1.0);
  double sum = 0.0;

  for (auto [alpha, info] : alphasMap) {
    sum += info.probability;

    if (rand <= sum)
      return alpha;
  }

  return 0;
}

Solution ReactiveGrasp::buildGreedyRandomizedSolution(double alpha)
{
  Solution solution;

  for (Vehicle *v : inst->vehicles)
    solution.routes.push_back(Route(v));

  for (Route &route : solution.routes) {
    route.path.push_back(inst->getOriginDepot());
    route.path.push_back(inst->getDestinationDepot());
  }

  struct Candidate
  {
    Route route;
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

    solution.setRoute(candidate->route.vehicle, candidate->route);

    // Save route to optimize the update of candidates
    Route selected = candidate->route;

    candidates.erase(candidate);

    // Update candidates
    for (Candidate &c : candidates)
      if (c.route == selected)
        c = {getCheapestFeasibleInsertion(c.request, solution), c.request};
  }

  return solution;
}

Route ReactiveGrasp::getCheapestFeasibleInsertion(Request req, Solution s)
{
  Route bestFeasible;
  Route bestInfeasible;

  bestFeasible.cost = MAXFLOAT;
  bestInfeasible.cost = MAXFLOAT;

  for (Route r : s.routes) {
    Route curr = getCheapestFeasibleInsertion(req, r);

    if (curr.feasible() && curr.cost < bestFeasible.cost)
      bestFeasible = curr;
    else if (!curr.feasible() && curr.cost < bestInfeasible.cost)
      bestInfeasible = curr;
  }

  // No feasible insertion was found
  if (bestFeasible.cost == MAXFLOAT)
    return bestInfeasible;

  return bestFeasible;
}

Route ReactiveGrasp::getCheapestFeasibleInsertion(Request req, Route r)
{
  // Best insertion starts with infinity cost, we will update it during the search
  Route bestFeasible = r;
  Route bestInfeasible = r;

  bestFeasible.cost = MAXFLOAT;
  bestInfeasible.cost = MAXFLOAT;

  // if (r.path[r.path.size() - 2]->isStation())
  //   r.path.erase(r.path.begin() + r.path.size() - 2);

  for (int p = 1; p < r.path.size(); p++) {
    // int lastEmptyPos = 1, load = 0, stationPos1 = -1;

    // for (int i = 1; i < p; i++) {
    //   if (r.path[i]->isPickup())
    //     load++;
    //   else if (r.path[i]->isDelivery())
    //     load--;

    //   if (load == 0)
    //     lastEmptyPos = i + 1;
    // }

    r.path.insert(r.path.begin() + p, req.pickup);

    // r.evaluate();

    // if (r.getStateOfCharge(p) < r.vehicle->finalMinStateOfCharge) {
    //   Node *station = inst->getNearestStation(r.path[lastEmptyPos - 1], r.path[lastEmptyPos]);
    //   r.path.insert(r.path.begin() + lastEmptyPos, station);
    //   stationPos1 = lastEmptyPos;
    //   p++;
    // }

    for (int d = p + 1; d < r.path.size(); d++) {
      r.path.insert(r.path.begin() + d, req.delivery);

      // bool stationBeforeDepot = false;

      // r.evaluate();

      // if (r.getStateOfCharge(r.path.size() - 1) < r.vehicle->finalMinStateOfCharge) {
      //   Node *station = inst->getNearestStation(r.path[r.path.size() - 2], r.path[r.path.size() - 1]);
      //   r.path.insert(r.path.begin() + r.path.size() - 1, station);
      //   stationBeforeDepot = true;
      // }

      r.evaluate();

      if (r.feasible() && r.cost < bestFeasible.cost)
        bestFeasible = r;
      else if (!r.feasible() && r.cost < bestInfeasible.cost)
        bestInfeasible = r;

      // if (stationBeforeDepot)
      //   r.path.erase(r.path.begin() + r.path.size() - 2);

      r.path.erase(r.path.begin() + d);
    }

    // if (stationPos1 != -1)
    //   r.path.erase(r.path.begin() + stationPos1);

    r.path.erase(r.path.begin() + p);
  }

  // No feasible insertion was found
  if (bestFeasible.cost == MAXFLOAT)
    return bestInfeasible;

  return bestFeasible;
}

Solution ReactiveGrasp::rvnd(Solution s, std::vector<Move> moves)
{
  std::vector<Move> rvndMoves = moves;

  while (!rvndMoves.empty()) {
    auto move = Random::get(rvndMoves); // Get an iterator to a random move
    Solution neighbor = (*move)(s);

    if (neighbor.objFuncValue() < s.objFuncValue()) {
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
  double bestObj = best.objFuncValue();

  for (Route r : s.routes) {
    for (Node *node : r.path) {
      if (node->isPickup()) {
        Request req = inst->getRequest(node);
        Route curr = r;

        // Erase request by value
        curr.path.erase(std::remove(curr.path.begin(), curr.path.end(), req.pickup),   curr.path.end());
        curr.path.erase(std::remove(curr.path.begin(), curr.path.end(), req.delivery), curr.path.end());

        // Reinsert the request
        curr = getCheapestFeasibleInsertion(req, curr);
        curr.evaluate();

        // Generate neighbor solution
        Solution neighbor = s;
        neighbor.setRoute(curr.vehicle, curr);
        double neighborObj = neighbor.objFuncValue();

        if (neighbor.feasible() && neighborObj < bestObj) {
          best = neighbor;
          bestObj = neighborObj;
        }
      }
    }
  }

  return best;
}

Solution ReactiveGrasp::swapZeroOne(Solution s)
{
  Solution best = s;
  double bestObj = best.objFuncValue();

  for (Route r1 : s.routes) {
    for (Node *node : r1.path) {
      if (node->isPickup()) {
        Request req = inst->getRequest(node);
        Route curr1 = r1;

        // Erase request by value
        curr1.path.erase(std::remove(curr1.path.begin(), curr1.path.end(), req.pickup),   curr1.path.end());
        curr1.path.erase(std::remove(curr1.path.begin(), curr1.path.end(), req.delivery), curr1.path.end());

        curr1.evaluate();

        for (Route r2 : s.routes) {
          if (r1 != r2) {
            // Insert req in the new route
            Route curr2 = getCheapestFeasibleInsertion(req, r2);
            curr2.evaluate();

            // Generate neighbor solution;
            Solution neighbor = s;
            neighbor.setRoute(curr1.vehicle, curr1);
            neighbor.setRoute(curr2.vehicle, curr2);
            double neighborObj = neighbor.objFuncValue();

            if (neighbor.feasible() && neighborObj < best.objFuncValue()) {
              best = neighbor;
              bestObj = neighborObj;
            }
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
    r1.evaluate();

    r2 = getCheapestFeasibleInsertion(req1, r2);
    r2.evaluate();

    Solution neighbor = s;
    neighbor.setRoute(r1.vehicle, r1);
    neighbor.setRoute(r2.vehicle, r2);

    if (neighbor.feasible() && neighbor.objFuncValue() < s.objFuncValue()) {
      s = neighbor;
      goto START;
    }
  }

  return s;
}

Solution ReactiveGrasp::removeStation(Solution s)
{
  Solution best = s;

  int c = 0;

  for (Route r : s.routes)
    for (Node *n : r.path)
      if (n->isStation())
        c++;

  std::cout << "Stations = " << c << '\n';

  for (Route r : s.routes) {
    for (int i = 1; i < r.path.size(); i++) {
      if (r.path[i]->isStation()) {
        Node *predecessor = r.path[i - 1];
        Node *successor = r.path[i + 1];

        double preToSuc = r.batteryLevels[i - 1] - r.vehicle->dischargingRate * inst->getTravelTime(predecessor, successor);

        // We have enough battery to go from predecessor to sucessor
        if (preToSuc/r.vehicle->batteryCapacity >= r.vehicle->finalMinStateOfCharge) {
          Route curr = r;
          curr.path.erase(curr.path.begin() + i);
          curr.evaluate();

          if (curr.feasible()) {
            // Generate neighbor solution
            Solution neighbor = s;
            neighbor.setRoute(curr.vehicle, curr);
            double neighborObj = neighbor.objFuncValue();

            if (neighborObj < best.objFuncValue()) {
              std::cout << "yo" << '\n';
              best = neighbor;
            }
          }
        }
      }
    }
  }

  return best;
}

void ReactiveGrasp::allocateStations(Solution &s)
{
  for (Route &r : s.routes) {
    r.evaluate();

    if (!r.feasible()) {
      int p = -1;

      for (int i = 0; i < r.path.size(); i++) {
        if (r.load[i] == 0 && r.getForwardTimeSlack(i) > 0) {
          p = i;
          // printf("\n(%d, %d) = %.2f", r.vehicle->id, i, r.getForwardTimeSlack(i));
        }
      }

      if (p != -1) {
        if (p == r.path.size() - 1)
          r.path.insert(r.path.begin() + p, inst->getNearestStation(r.path[p], r.path[p - 1]));
        else
          r.path.insert(r.path.begin() + p + 1, inst->getNearestStation(r.path[p], r.path[p + 1]));

        r.evaluate();
      }
    }
  }
}

void ReactiveGrasp::updateProbabilities(std::map<double, AlphaInfo> &alphasMap, double bestCost)
{
  double qsum = 0.0;

  for (auto [alpha, info] : alphasMap)
    if (info.avg() > 0)
      qsum += bestCost/info.avg();

  for (auto &[alpha, info] : alphasMap)
    info.probability = (bestCost/info.avg())/qsum;
}
