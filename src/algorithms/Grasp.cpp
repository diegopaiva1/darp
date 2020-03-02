/**
 * @file   Grasp.cpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include <algorithm>

#include "Grasp.hpp"

Singleton *instance = Singleton::getInstance();

Solution Grasp::solve(int iterations = 100, int blocks = 10, std::vector<double> alphas = {0.5, 1.0})
{
  Solution best;

  // GRASP components
  int n = alphas.size();
  std::vector<int>    counter (n, 0);
  std::vector<double> probabilities (10, 1.0/n);
  std::vector<double> costs (n, 0.0);
  std::vector<double> q (n, 0.0);

  for (int it = 1; it <= iterations; it++) {
    int index;
    int alphaIndex;
    Solution currSolution;
    std::vector<Request> requests;

    if (it != 1) {
      alphaIndex = chooseAlphaIndex(probabilities);
      counter[alphaIndex]++;
    }

    if (it % blocks == 0)
      updateProbabilities(probabilities, q);

    for (Request &req : instance->requests)
      requests.push_back(req);

    for (Vehicle &v : instance->vehicles)
      currSolution.routes.push_back(Route(v));

    for (Route &route : currSolution.routes) {
      route.path.push_back(instance->getOriginDepot());
      route.path.push_back(instance->getDestinationDepot());
    }

    while (!requests.empty()) {
      if (it == 1)
        index = 0;
      else
        index = std::get<0>(Prng::generateInteger(0, (int) (alphas[alphaIndex] * requests.size() - 1)));

      Request request = requests[index];
      Route bestRoute;

      for (int i = 0; i < currSolution.routes.size(); i++) {
        Route r = performCheapestFeasibleInsertion(request, currSolution.routes[i]);

        if (i == 0 || r.cost < bestRoute.cost)
          bestRoute = r;
      }

      // Request could not be feasibly inserted, so we create a new route (thus solution will be infeasible)
      if (bestRoute.cost == MAXFLOAT) {
        Route newest = createRoute(currSolution);
        newest.path.push_back(instance->getOriginDepot());
        newest.path.push_back(request.pickup);
        newest.path.push_back(request.delivery);
        newest.path.push_back(instance->getDestinationDepot());
        newest.performEightStepEvaluationScheme();
        currSolution.routes.push_back(newest);
      }
      else {
        currSolution.routes.at(bestRoute.vehicle.id - 1) = bestRoute;
      }

      requests.erase(requests.begin() + index);
    }

    currSolution.computeCost();
    currSolution = localSearch(currSolution);

    if ((it == 1) || (currSolution.routes.size() < best.routes.size()) ||
        (currSolution.routes.size() == best.routes.size() && currSolution.cost < best.cost))
      best = currSolution;

    if (it != 1) {
      int param1 = 0;
      int param2 = 0;

      if (currSolution.routes.size() > instance->vehicles.size())
        param1 = 1000;

      if (best.routes.size() > instance->vehicles.size())
        param2 = 1000;

      costs[alphaIndex] += currSolution.cost + param1 * currSolution.routes.size();
      q[alphaIndex] = (best.cost + param2 * best.routes.size())/(costs[alphaIndex]/counter[alphaIndex]);

      printf("\ns* = %.2f e a[%d] = %.2f (%d)", best.cost, alphaIndex, costs[alphaIndex]/counter[alphaIndex], it);
    }

    if (it == iterations) {
      printf("\n\nAlphas:\n");

      for (int p = 0; p < probabilities.size(); p++)
        printf("%d. %.2f (%.2f%%) - Escolhido %d vezes\n", p + 1, alphas[p], probabilities[p], counter[p]);
    }
  }

  return best;
}

Solution Grasp::localSearch(Solution s)
{
  std::vector<int> neighborhoods = {1, 2, 3};

  while (!neighborhoods.empty()) {
    Solution neighbor;
    int k = std::get<0>(Prng::generateInteger(0, (int) neighborhoods.size() - 1));

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

Solution Grasp::_3opt(Solution s)
{
  Solution best = s;

  for (int k = 0; k < s.routes.size(); k++) {
    if (s.routes[k].path.size() >= 3) {
      for (int i = 0; i < s.routes[k].path.size() - 3; i++) {
        for (int j = i + 1; j < s.routes[k].path.size() - 2; j++) {
          for (int n = j + 1; n < s.routes[k].path.size() - 1; n++) {
            Route r = s.routes[k];

            double d0 = instance->getTravelTime(r.path[i], r.path[i + 1]) +
                        instance->getTravelTime(r.path[j], r.path[j + 1]) +
                        instance->getTravelTime(r.path[n], r.path[n + 1]);

            double d1 = instance->getTravelTime(r.path[i], r.path[i + 1]) +
                        instance->getTravelTime(r.path[j], r.path[n]) +
                        instance->getTravelTime(r.path[j + 1], r.path[n + 1]);

            double d2 = instance->getTravelTime(r.path[i], r.path[j]) +
                        instance->getTravelTime(r.path[i + 1], r.path[j + 1]) +
                        instance->getTravelTime(r.path[n], r.path[n + 1]);

            double d3 = instance->getTravelTime(r.path[i], r.path[j]) +
                        instance->getTravelTime(r.path[i + 1], r.path[n]) +
                        instance->getTravelTime(r.path[j + 1], r.path[n + 1]);

            double d4 = instance->getTravelTime(r.path[i], r.path[j + 1]) +
                        instance->getTravelTime(r.path[i + 1], r.path[n]) +
                        instance->getTravelTime(r.path[j], r.path[n + 1]);

            double d5 = instance->getTravelTime(r.path[i], r.path[j + 1]) +
                        instance->getTravelTime(r.path[i + 1], r.path[n + 1]) +
                        instance->getTravelTime(r.path[j], r.path[n]);

            double d6 = instance->getTravelTime(r.path[i], r.path[n]) +
                        instance->getTravelTime(r.path[i + 1], r.path[j + 1]) +
                        instance->getTravelTime(r.path[j], r.path[n + 1]);

            double d7 = instance->getTravelTime(r.path[i], r.path[n]) +
                        instance->getTravelTime(r.path[i + 1], r.path[n + 1]) +
                        instance->getTravelTime(r.path[j], r.path[j + 1]);

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

            if (r.isFeasible() && *r.path.begin() == instance->getOriginDepot() &&
                *r.path.end() == instance->getDestinationDepot() && r.cost < best.routes[k].cost) {
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

Solution Grasp::_2opt(Solution s)
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

Solution Grasp::swapZeroOne(Solution s)
{
  Solution best = s;

  for (int k1 = 0; k1 < s.routes.size(); k1++) {
    for (Node *p : s.routes[k1].path) {
      if (p->isPickup()) {
        Route r1 = s.routes[k1];
        Request req = instance->getRequest(p);

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

Solution Grasp::relocate(Solution s)
{
  Solution best = s;

  for (int k = 0; k < s.routes.size(); k++) {
    for (Node *p : s.routes[k].path) {
      if (p->isPickup()) {
        Request req = instance->getRequest(p);
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

Route Grasp::createRoute(Solution &s)
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

int Grasp::chooseAlphaIndex(std::vector<double> probabilities)
{
  double rand = std::get<0>(Prng::generateDouble(0.0, 1.0));
  double sum = 0.0;

  for (int i = 0; i < probabilities.size(); i++) {
    sum += probabilities[i];

    if (rand <= sum)
      return i;
  }

  return 0;
}

void Grasp::updateProbabilities(std::vector<double> &probabilities, std::vector<double> q)
{
  for (int i = 0; i < probabilities.size(); i++)
    probabilities[i] = q[i]/std::accumulate(q.begin(), q.end(), 0.0);
}

Route Grasp::performCheapestFeasibleInsertion(Request req, Route r)
{
  // Best insertion starts with infinity cost, we will update it during the search
  Route best = r;
  best.cost = MAXFLOAT;

  for (int p = 1; p < r.path.size(); p++) {
    r.path.insert(r.path.begin() + p, req.pickup);

    for (int d = p + 1; d < r.path.size(); d++) {
      r.path.insert(r.path.begin() + d, req.delivery);
      r.performEightStepEvaluationScheme();

      // if (r.batteryLevelViolation) {
      //   for (int i = 0; i < r.path.size(); i++) {
      //     if (r.batteryLevels[i] < 0) {
      //       for (int j = 0; j <= i; j++) {
      //         if (r.load[j] == 0) {
      //           Node *station = instance->getNearestStation(r.path[j], r.path[j + 1]);

      //           r.path.insert(r.path.begin() + j + 1, station);
      //           r.performEightStepEvaluationScheme();

      //           if (r.batteryLevels[i] > 0) {
      //             // Solved the battery issue, we can leave the inner loop
      //             break;
      //           }
      //           else {
      //             // Issue not solved, just erase the inserted station and continue the search
      //             r.path.erase(r.path.begin() + j + 1);
      //           }
      //         }
      //       }
      //     }
      //   }
      // }

      // if (r.finalBatteryViolation) {
      //   for (int i = 0; i < r.path.size(); i++) {
      //     if (r.batteryLevels[i] < r.vehicle.minFinalBatteryRatioLevel * r.vehicle.batteryCapacity) {
      //       Node *station = instance->getNearestStation(r.path[i - 1], r.path[i]);

      //       r.path.insert(r.path.begin() + i, station);
      //       r.performEightStepEvaluationScheme();
      //     }
      //   }
      // }

      if (r.isFeasible() && r.cost < best.cost)
        best = r;

      // for (Node *n : r.path)
      //   if (n->isStation() || n == req.delivery)
      //     r.path.erase(std::remove(r.path.begin(), r.path.end(), n), r.path.end());

      r.path.erase(r.path.begin() + d);
    }

    r.path.erase(r.path.begin() + p);
  }

  return best;
}
