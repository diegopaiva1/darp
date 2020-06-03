/**
 * @file   ReactiveGrasp.cpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "data-structures/Singleton.hpp"
#include "algorithms/ReactiveGrasp.hpp"
#include "utils/Timer.hpp"
#include "utils/Display.hpp"

#include <iomanip>

Run ReactiveGrasp::solve(int iterations, int blocks, std::vector<double> alphas)
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
  std::vector<Move> moves = {reinsert, swapZeroOne, swapOneOne};

  Solution best;
  double bestObj = MAXFLOAT;
  double bestAlpha;
  int bestIteration;

  for (int it = 0; it <= iterations; it++) {
    // Reserve a first iteration to go full greedy
    double alpha = it == 0 ? 0.0 : getRandomAlpha(alphasMap);

    Solution curr = buildGreedyRandomizedSolution(alpha);
    curr = rvnd(curr, moves);

    double currObj = curr.obj();

    if (it == 0 || (curr.feasible() && (currObj < bestObj || !best.feasible()))) {
      best = curr;
      bestObj = currObj;
      bestIteration = it;
      bestAlpha = alpha;
    }

    // Remember: first iteration is full greedy, so no need to update alpha info
    if (it != 0) {
      int penalty = !curr.feasible() ? 10 : 1; // Penalize alphas that generated infeasible solutions

      alphasMap[alpha].count++;
      alphasMap[alpha].sum += currObj * penalty;

      if (it % blocks == 0)
        updateProbabilities(alphasMap, bestObj);
    }

    // for (auto [alpha, info] : alphasMap)
    //   printf("\nAvg %.2f = %.2f (%.2f)", alpha, info.avg(), info.probability);

    Display::printProgress(best.feasible(), bestObj, (double) it/iterations);
  }

  // Erase any route without requests from best
  for (auto r = best.routes.begin(); r != best.routes.end(); )
    r = (r->empty()) ? best.routes.erase(r) : r + 1;

  for (auto [alpha, info] : alphasMap)
    printf("%.2f - %d times (%.2f%%)\n", alpha, info.count, info.probability);

  std::map<double, double> probDistribution;

  for (auto [alpha, info] : alphasMap)
    probDistribution[alpha] = info.probability;

  return Run(best, timer.elapsedMinutes(), seed, bestIteration, bestAlpha, probDistribution);
}

double ReactiveGrasp::getRandomAlpha(std::map<double, AlphaInfo> alphasMap)
{
  double rand = Random::get(0.0, 1.0);
  double sum  = 0.0;

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

  bestFeasible.cost   = MAXFLOAT;
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
  Route bestFeasible   = r;
  Route bestInfeasible = r;

  bestFeasible.cost   = MAXFLOAT;
  bestInfeasible.cost = MAXFLOAT;

  if (r.path[r.path.size() - 2]->isStation())
    r.path.erase(r.path.begin() + r.path.size() - 2);

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

    // if (r.getStateOfCharge(p) < 0.0) {
    //   Node *station = inst->getNearestStation(r.path[lastEmptyPos - 1], r.path[lastEmptyPos]);
    //   r.path.insert(r.path.begin() + lastEmptyPos, station);
    //   stationPos1 = lastEmptyPos;
    // }

    for (int d = p + 1; d < r.path.size(); d++) {
      r.path.insert(r.path.begin() + d, req.delivery);

      bool stationBeforeDepot = false;

      if (r.getStateOfCharge(r.path.size() - 1) < r.vehicle->finalMinStateOfCharge) {
        Node *station = inst->getNearestStation(r.path[r.path.size() - 2], r.path[r.path.size() - 1]);
        r.path.insert(r.path.begin() + r.path.size() - 1, station);
        stationBeforeDepot = true;
      }

      r.evaluate();

      if (r.feasible() && r.cost < bestFeasible.cost)
        bestFeasible = r;
      else if (!r.feasible() && r.cost < bestInfeasible.cost)
        bestInfeasible = r;

      if (stationBeforeDepot)
        r.path.erase(r.path.begin() + r.path.size() - 2);

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

    if (neighbor.obj() < s.obj()) {
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
  double bestObj = best.obj();

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
        double neighborObj = neighbor.obj();

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
  double bestObj = best.obj();

  for (Route r1 : s.routes) {
    for (Node *node : r1.path) {
      if (node->isPickup()) {
        Request req   = inst->getRequest(node);
        Route   curr1 = r1;

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
            double neighborObj = neighbor.obj();

            if (neighbor.feasible() && neighborObj < best.obj()) {
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

    if (neighbor.feasible() && neighbor.obj() < s.obj()) {
      s = neighbor;
      goto START;
    }
  }

  return s;
}

// Solution ReactiveGrasp::removeStation(Solution s)
// {
//   Solution best = s;

//   for (Route r : s.routes) {
//     for (auto nodeIt = r.path.begin(); nodeIt != r.path.end(); nodeIt++) {
//       if ((*nodeIt)->isStation()) {
//         Node *predecessor = *(nodeIt - 1);
//         Node *successor   = *(nodeIt + 1);

//         int index = nodeIt - r.path.begin();

//         double preToSuc = r.batteryLevels[index - 1] - r.vehicle.dischargingRate * inst->getTravelTime(predecessor, successor);

//         // We have enough battery to go from predecessor to sucessor
//         if (!successor->isDepot() && preToSuc > 0.0) {
//           Route curr = r;
//           curr.path.erase(curr.path.begin() + index);

//           if (curr.feasible()) {
//             // Generate neighbor solution
//             Solution neighbor = s;
//             neighbor.setRoute(curr.vehicle, curr);
//             neighbor.updateCost();

//             if (neighbor.cost < best.cost)
//               best = neighbor;
//           }
//         }
//       }
//     }
//   }

//   return best;
// }

void ReactiveGrasp::updateProbabilities(std::map<double, AlphaInfo> &alphasMap, double bestCost)
{
  double qsum = 0.0;

  for (auto [alpha, info] : alphasMap)
    if (info.avg() > 0)
      qsum += bestCost/info.avg();

  for (auto &[alpha, info] : alphasMap)
    info.probability = (bestCost/info.avg())/qsum;
}
