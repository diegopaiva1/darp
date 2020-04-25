/**
 * @file   ReactiveGrasp.cpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include <algorithm>

#include "data-structures/Singleton.hpp"
#include "algorithms/ReactiveGrasp.hpp"
#include "utils/Timer.hpp"
#include "utils/Display.hpp"

std::tuple<Solution, double, uint, int>
ReactiveGrasp::solve(int iterations = 100, int blocks = 10, std::vector<double> alphas = {1.0})
{
  // Use std::random_device to generate seed to Random engine
  int seed = std::random_device{}();
  Random::seed(seed);

  // Starting a clock to count algorithm's run time
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
  int optimalIteration = 0;

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

    if (it == 1 || curr.routes.size() < best.routes.size() || (curr.routes.size() == best.routes.size() && curr.cost < best.cost)) {
      best = curr;

      if (optimalIteration == 0 && fabs(best.cost - inst->optimalSolutions[inst->name]) < 0.01)
        optimalIteration = it;
    }

    if (it % blocks == 0) {
      for (int i = 0; i < randomParams.size(); i++) {
        double avg = randomParams[i].count > 0 ? randomParams[i].cumulativeCost/randomParams[i].count : 0;

        if (avg > 0)
          randomParams[i].q = best.cost/avg;
      }

      updateProbabilities(randomParams);
    }

    // Erase any route without requests
    for (auto k = curr.routes.begin(); k != curr.routes.end(); )
      k = (k->path.size() <= 2) ? curr.routes.erase(k) : k + 1;

    if (it != 1) {
      int param = curr.routes.size() > inst->vehicles.size() ? 1000 : 0;
      randomParams[index].cumulativeCost += curr.cost + param * curr.routes.size();
    }

    // printf("\n");
    // for (int i = 0; i < randomParams.size(); i++) {
    //   double avg = randomParams[i].count > 0 ? randomParams[i].cumulativeCost/randomParams[i].count : 0;
    //   printf("Avg %.2f = %.2f (%.2f)\n", randomParams[i].alpha, avg, randomParams[i].probability);
    // }

    Display::printProgress(best, (double) it/iterations);
  }

  double elapsedTime = timer.elapsedInMinutes();

  Display::printSolutionInfoWithElapsedTime(best, elapsedTime);

  for (int i = 0; i < randomParams.size(); i++) {
    RandomParam rp = randomParams[i];
    printf("%2d. %.2f - %d times (%.2f%%)\n", i + 1, rp.alpha, rp.count, rp.probability);
  }

  printf("%d\n", optimalIteration);

  return std::make_tuple(best, elapsedTime, seed, optimalIteration);
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

    int index = Random::get(0, (int) (alpha * (candidates.size() - 1)));
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
    int k = Random::get(0, (int) neighborhoods.size() - 1);

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

            if (r1.isFeasible() && r2.isFeasible() && r1.cost + r2.cost < s.routes[k1].cost + s.routes[k2].cost) {
              s.routes[k1] = r1;
              s.routes[k2] = r2;
            }
          }
        }
      }
    }
  }

  s.computeCost();

  return s;
}

Solution ReactiveGrasp::relocate(Solution s)
{
  for (int k = 0; k < s.routes.size(); k++) {
    for (int i = 1; i < s.routes[k].path.size() - 1; i++) {
      Node *p = s.routes[k].path[i];

      if (p->isPickup()) {
        Request req = inst->getRequest(p);
        Route r = s.routes[k];

        r.path.erase(std::remove(r.path.begin(), r.path.end(), req.pickup),   r.path.end());
        r.path.erase(std::remove(r.path.begin(), r.path.end(), req.delivery), r.path.end());

        r = performCheapestFeasibleInsertion(req, r);

        if (r.isFeasible() && r.cost < s.routes[k].cost) {
          s.routes[k] = r;
          i = 1;
        }
      }
    }
  }

  s.computeCost();

  return s;
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
  double rand = Random::get(0.0, 1.0);
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
