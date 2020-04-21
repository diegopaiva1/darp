/**
 * @file   ReactiveGrasp.cpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include <algorithm>

#include "data-structures/Singleton.hpp"
#include "algorithms/ReactiveGrasp.hpp"
#include "utils/Timer.hpp"
#include "utils/Prng.hpp"
#include "utils/Display.hpp"

std::pair<Solution, double> ReactiveGrasp::solve(int iterations = 100, int blocks = 10, std::vector<double> alphas = {1.0})
{
  // Starting a clock to count run time
  Timer timer;

  std::vector<RandomParam> randomParams (alphas.size());

  // Initializing each alpha as random parameter
  for (int i = 0; i < alphas.size(); i++) {
    RandomParam rp;

    rp.alpha          = alphas[i];
    rp.probability    = 1.0/alphas.size();
    rp.q              = 0.0;
    rp.cumulativeCost = 0.0;
    rp.count          = 0;

    randomParams[i] = rp;
  }

  Solution best;

  for (int it = 1; it <= iterations; it++) {
    int index;
    double alpha;

    // Go full greedy at first iteration
    if (it == 1) {
      alpha = 0.0;
    }
    else {
      index = chooseRandomParamIndex(randomParams);
      alpha = randomParams[index].alpha;
      randomParams[index].count++;
    }

    Solution curr = buildGreedyRandomizedSolution(alpha);
    curr = rvnd(curr);

    if (it == 1 || curr.routes.size() < best.routes.size() || (curr.routes.size() == best.routes.size() && curr.cost < best.cost))
      best = curr;

    if (it % blocks == 0)
      updateProbabilities(randomParams);

    if (it != 1) {
      int param1 = curr.routes.size() > inst->vehicles.size() ? 1000 : 0;
      int param2 = best.routes.size() > inst->vehicles.size() ? 1000 : 0;

      randomParams[index].cumulativeCost += curr.cost + param1 * curr.routes.size();
      randomParams[index].q = (best.cost + param2 * best.routes.size())/
                              (randomParams[index].cumulativeCost/randomParams[index].count);
    }

    Display::printProgress(best, (double) it/iterations);
  }

  double elapsedTime = timer.elapsedInMinutes();

  // Erase any route without requests
  for (auto k = best.routes.begin(); k != best.routes.end(); )
    k = (k->path.size() <= 2) ? best.routes.erase(k) : k + 1;

  Display::printSolutionInfoWithElapsedTime(best, elapsedTime);

  return std::make_pair(best, elapsedTime);
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

  struct Candidate {
    Route   route;
    Request request;
  };

  std::vector<Candidate> candidates;

  // Initialize candidates
  for (Request req : inst->requests)
    candidates.push_back({getBestInsertion(req, solution), req});

  while (!candidates.empty()) {
    std::sort(candidates.begin(), candidates.end(), [] (Candidate &c1, Candidate &c2) {
      return c1.route.cost < c2.route.cost;
    });

    int index = Prng::generateInteger(0, (int) (alpha * (candidates.size() - 1))).first;
    Candidate chosen = candidates[index];

    // Request could not be feasibly inserted, so we create a new route (thus solution will be infeasible)
    if (chosen.route.cost == MAXFLOAT) {
      Route newest = createRoute(solution);
      newest.path.push_back(inst->getOriginDepot());
      newest.path.push_back(chosen.request.pickup);
      newest.path.push_back(chosen.request.delivery);
      newest.path.push_back(inst->getDestinationDepot());
      solution.routes.push_back(newest);
    }
    else {
      solution.routes.at(chosen.route.vehicle.id - 1) = chosen.route;
    }

    candidates.erase(candidates.begin() + index);

    // Update candidates
    for (int i = 0; i < candidates.size(); i++)
      if (candidates[i].route.vehicle.id == chosen.route.vehicle.id)
        candidates[i] = {getBestInsertion(candidates[i].request, solution), candidates[i].request};
  }

  solution.computeCost();

  return solution;
}

Solution ReactiveGrasp::rvnd(Solution s)
{
  std::vector<int> neighborhoods = {1, 2, 3};

  while (!neighborhoods.empty()) {
    Solution neighbor;
    int k = Prng::generateInteger(0, (int) neighborhoods.size() - 1).first;

    switch (neighborhoods.at(k)) {
      case 1:
        neighbor = relocate(s);
        break;
      case 2:
        neighbor = _3opt(s);
        break;
      case 3:
        neighbor = swapZeroOne(s);
        break;
    }

    if (neighbor.cost < s.cost) {
      s = neighbor;
      neighborhoods = {1, 2, 3};
    }
    else {
      neighborhoods.erase(neighborhoods.begin() + k);
    }
  }

  return s;
}

Solution ReactiveGrasp::_3opt(Solution s)
{
  Solution best = s;

  for (int k = 0; k < s.routes.size(); k++) {
    if (s.routes[k].path.size() >= 3) {
      for (int i = 0; i < s.routes[k].path.size() - 3; i++) {
        for (int j = i + 1; j < s.routes[k].path.size() - 2; j++) {
          for (int n = j + 1; n < s.routes[k].path.size() - 1; n++) {
            Route r = s.routes[k];

            double d0 = inst->getTravelTime(r.path[i], r.path[i + 1]) +
                        inst->getTravelTime(r.path[j], r.path[j + 1]) +
                        inst->getTravelTime(r.path[n], r.path[n + 1]);

            double d1 = inst->getTravelTime(r.path[i], r.path[i + 1]) +
                        inst->getTravelTime(r.path[j], r.path[n]) +
                        inst->getTravelTime(r.path[j + 1], r.path[n + 1]);

            double d2 = inst->getTravelTime(r.path[i], r.path[j]) +
                        inst->getTravelTime(r.path[i + 1], r.path[j + 1]) +
                        inst->getTravelTime(r.path[n], r.path[n + 1]);

            double d3 = inst->getTravelTime(r.path[i], r.path[j]) +
                        inst->getTravelTime(r.path[i + 1], r.path[n]) +
                        inst->getTravelTime(r.path[j + 1], r.path[n + 1]);

            double d4 = inst->getTravelTime(r.path[i], r.path[j + 1]) +
                        inst->getTravelTime(r.path[i + 1], r.path[n]) +
                        inst->getTravelTime(r.path[j], r.path[n + 1]);

            double d5 = inst->getTravelTime(r.path[i], r.path[j + 1]) +
                        inst->getTravelTime(r.path[i + 1], r.path[n + 1]) +
                        inst->getTravelTime(r.path[j], r.path[n]);

            double d6 = inst->getTravelTime(r.path[i], r.path[n]) +
                        inst->getTravelTime(r.path[i + 1], r.path[j + 1]) +
                        inst->getTravelTime(r.path[j], r.path[n + 1]);

            double d7 = inst->getTravelTime(r.path[i], r.path[n]) +
                        inst->getTravelTime(r.path[i + 1], r.path[n + 1]) +
                        inst->getTravelTime(r.path[j], r.path[j + 1]);

            if (d1 < d0) {
              r = s.routes[k];
              std::swap(r.path[j + 1], r.path[n]);
              r.performEightStepEvaluationScheme();
            }
            else if (d2 < d0 || !r.isFeasible()) {
              r = s.routes[k];
              std::swap(r.path[i + 1], r.path[j]);
              r.performEightStepEvaluationScheme();
            }
            else if (d3 < d0 || !r.isFeasible()) {
              r = s.routes[k];
              std::swap(r.path[i + 1], r.path[j]);
              std::swap(r.path[j + 1], r.path[n]);
              r.performEightStepEvaluationScheme();
            }
            else if (d4 < d0 || !r.isFeasible()) {
              r = s.routes[k];
              std::swap(r.path[i + 1], r.path[j + 1]);
              std::swap(r.path[j + 1], r.path[j]);
              std::swap(r.path[j + 1], r.path[n]);
              r.performEightStepEvaluationScheme();
            }
            else if (d5 < d0 || !r.isFeasible()) {
              r = s.routes[k];
              std::swap(r.path[i + 1], r.path[j + 1]);
              std::swap(r.path[j + 1], r.path[j]);
              std::swap(r.path[j + 1], r.path[n]);
              std::swap(r.path[j + 1], r.path[n + 1]);
              r.performEightStepEvaluationScheme();
            }
            else if (d6 < d0 || !r.isFeasible()) {
              r = s.routes[k];
              std::swap(r.path[i + 1], r.path[j]);
              std::swap(r.path[i + 1], r.path[n]);
              r.performEightStepEvaluationScheme();
            }
            else if (d7 < d0 || !r.isFeasible()) {
              r = s.routes[k];
              std::swap(r.path[i + 1], r.path[j]);
              std::swap(r.path[i + 1], r.path[n]);
              std::swap(r.path[j + 1], r.path[n + 1]);
              r.performEightStepEvaluationScheme();
            }

            if (r.isFeasible() && *r.path.begin() == inst->getOriginDepot() &&
                *r.path.end() == inst->getDestinationDepot() && r.cost < best.routes[k].cost) {
              best.routes[k] = r;
            }
          }
        }
      }
    }
  }

  best.computeCost();

  return best;
}

Solution ReactiveGrasp::_2opt(Solution s)
{
  Solution best = s;

  for (int k = 0; k < s.routes.size(); k++) {
    for (int i = 1; i < s.routes[k].path.size() - 2; i++) {
      for (int j = i + 1; j < s.routes[k].path.size() - 1; j++) {
        Route r = s.routes[k];
        std::reverse(r.path.begin() + i, r.path.begin() + j + 1);
        r.performEightStepEvaluationScheme();

        if (r.isFeasible() && r.cost < best.routes[k].cost) {
          best.routes[k] = r;
        }
      }
    }
  }

  best.computeCost();

  return s;
}

Solution ReactiveGrasp::swapZeroOne(Solution s)
{
  Solution best = s;

  for (int k1 = 0; k1 < s.routes.size(); k1++) {
    for (Node *p : s.routes[k1].path) {
      if (p->isPickup()) {
        Route r1 = s.routes[k1];
        Request req = inst->getRequest(p);

        r1.path.erase(std::remove(r1.path.begin(), r1.path.end(), req.pickup),   r1.path.end());
        r1.path.erase(std::remove(r1.path.begin(), r1.path.end(), req.delivery), r1.path.end());
        r1.performEightStepEvaluationScheme();

        for (int k2 = 0; k2 < s.routes.size(); k2++) {
          if (k1 != k2) {
            Route r2 = performCheapestFeasibleInsertion(req, s.routes[k2]);

            if (r1.isFeasible() && r2.cost != MAXFLOAT && r1.cost + r2.cost < best.routes[k1].cost + best.routes[k2].cost) {
              best = s;
              best.routes[k1] = r1;
              best.routes[k2] = r2;
            }
          }
        }
      }
    }
  }

  best.computeCost();

  return best;
}

Solution ReactiveGrasp::relocate(Solution s)
{
  Solution best = s;

  for (int k = 0; k < s.routes.size(); k++) {
    for (Node *p : s.routes[k].path) {
      if (p->isPickup()) {
        Request req = inst->getRequest(p);
        Route r = s.routes[k];

        r.path.erase(std::remove(r.path.begin(), r.path.end(), req.pickup),   r.path.end());
        r.path.erase(std::remove(r.path.begin(), r.path.end(), req.delivery), r.path.end());

        r = performCheapestFeasibleInsertion(req, r);

        if (r.cost != MAXFLOAT && r.cost < best.routes[k].cost) {
          best.routes[k] = r;
        }
      }
    }
  }

  best.computeCost();

  return best;
}

Route ReactiveGrasp::createRoute(Solution &s)
{
  Vehicle newest(s.routes.size() + 1);
  Vehicle v = s.routes.at(0).vehicle;

  newest.batteryCapacity           = v.batteryCapacity;
  newest.capacity                  = v.capacity;
  newest.dischargingRate           = v.dischargingRate;
  newest.initialBatteryLevel       = v.initialBatteryLevel;
  newest.minFinalBatteryRatioLevel = v.minFinalBatteryRatioLevel;

  return Route(newest);
}

Route ReactiveGrasp::getBestInsertion(Request request, Solution solution)
{
  Route best;

  for (int k = 0; k < solution.routes.size(); k++) {
    Route r = performCheapestFeasibleInsertion(request, solution.routes[k]);

    if (k == 0 || r.cost < best.cost)
      best = r;
  }

  return best;
}

Route ReactiveGrasp::performCheapestFeasibleInsertion(Request req, Route r)
{
  // Best insertion starts with infinity cost, we will update it during the search
  Route best = r;
  best.cost = MAXFLOAT;

  for (int p = 1; p < r.path.size(); p++) {
    r.path.insert(r.path.begin() + p, req.pickup);

    for (int d = p + 1; d < r.path.size(); d++) {
      r.path.insert(r.path.begin() + d, req.delivery);
      r.performEightStepEvaluationScheme();

      if (r.isFeasible() && r.cost < best.cost)
        best = r;

      r.path.erase(r.path.begin() + d);
    }

    r.path.erase(r.path.begin() + p);
  }

  return best;
}

int ReactiveGrasp::chooseRandomParamIndex(std::vector<RandomParam> randomParams)
{
  double rand = Prng::generateDouble(0.0, 1.0).first;
  double sum  = 0.0;

  for (int i = 0; i < randomParams.size(); i++) {
    sum += randomParams[i].probability;

    if (rand <= sum)
      return i;
  }

  return 0;
}

void ReactiveGrasp::updateProbabilities(std::vector<RandomParam> &randomParams)
{
  double qsum = 0.0;

  for (RandomParam rp : randomParams)
    qsum += rp.q;

  for (int i = 0; i < randomParams.size(); i++)
    randomParams[i].probability = randomParams[i].q/qsum;
}
