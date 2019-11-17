/**
 * @file   Grasp.cpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "Grasp.hpp"
#include <algorithm>

Singleton *instance = Singleton::getInstance();

Solution Grasp::solve(int iterations = 1000, int iterationBlocks = 100, std::vector<float> alphas = {0.1, 0.2, 0.3, 0.4, 0.5})
{
  Solution best;

  // GRASP components
  int n = alphas.size();
  std::vector<int>    counter (n, 0);
  std::vector<double> probabilities (10, 1.0/n);
  std::vector<double> costs (n, 0.0);
  std::vector<double> q (n, 0.0);

  // In the beginning of the search, all penalty parameters are initialized with 1.0
  std::vector<float> penaltyParams = {1.0, 1.0, 1.0, 1.0, 1.0};

  // Random value that helps adjusting the penalty parameters dinamically
  float delta = Prng::generateFloatInRange(0.05, 0.10);

  for (int it = 1; it <= iterations; it++) {
    int index;
    int alphaIndex = Prng::generateIntegerInRange(0, alphas.size() - 1);
    Solution currSolution;
    std::vector<Request> requests;

    if (it != 1) {
      alphaIndex = chooseAlphaIndex(probabilities);
      counter[alphaIndex]++;
    }

    if (it % iterationBlocks == 0)
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
        index = Prng::generateIntegerInRange(0, (int) (alphas[alphaIndex] * requests.size() - 1));

      Request request = requests[index];
      Route bestRoute;

      for (int i = 0; i < currSolution.routes.size(); i++) {
        Route r = performCheapestFeasibleInsertion(request, currSolution.routes[i]);

        if (i == 0 || r.cost < bestRoute.cost)
          bestRoute = r;
      }

      // Request could not be feasibly inserted, so we create a new route (thus solution will be infeasible)
      if (bestRoute.cost == MAX_FLOAT) {
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

    currSolution.computeCost(penaltyParams);
    currSolution = localSearch(currSolution, penaltyParams);
    adjustPenaltyParams(currSolution, penaltyParams, delta);

   /* Everytime a new incumbent solution is found, we randomly choose a new delta. According to
    * (Parragh et. al, 2010) this works as a diversification mecanism and avoids cycling
    */
    delta = Prng::generateFloatInRange(0.05, 0.10);

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

      printf("\ns* = %.2f e a[%d] = %.2f", best.cost, alphaIndex, costs[alphaIndex]/counter[alphaIndex]);
    }

    if (it == iterations) {
      printf("\n\nAlphas:\n");
      for (int p = 0; p < probabilities.size(); p++)
        printf("%d. %.2f (%.2f%%) - Escolhido %d vezes\n", p + 1, alphas[p], probabilities[p], counter[p]);
    }
  }

  return best;
}

Solution Grasp::localSearch(Solution s, std::vector<float> penaltyParams)
{
  std::vector<int> neighborhoods = {2, 2};

  while (!neighborhoods.empty()) {
    Solution neighbor;
    int index = Prng::generateIntegerInRange(0, neighborhoods.size() - 1);

    switch (neighborhoods.at(index)) {
      case 1:
        neighbor = relocate(s, penaltyParams);
        break;
      case 2:
        neighbor = eliminate(s, penaltyParams);
        break;
      case 3:
        neighbor = _2opt(s, penaltyParams);
        break;
    }

    if (neighbor.cost < s.cost) {
      s = neighbor;
      neighborhoods = {2, 2};
    }
    else {
      neighborhoods.erase(neighborhoods.begin() + index);
    }
  }

  return s;
}

Solution Grasp::eliminate(Solution s, std::vector<float> penaltyParams)
{
  Solution original = s;
  Route random = s.routes.at(0);
  int c = 0;

  for (Node *p : random.path)
    if (p->isPickup())
      c++;

  int insertedUsers = 0;

  for (Node *p : random.path) {
    if (p->isPickup()) {
      Request req = instance->getRequest(p);
      bool inserted = false;
      Route at;

      for (int k = 0; k < s.routes.size(); k++) {
        if (s.routes[k].vehicle.id != random.vehicle.id) {
          Route r = performCheapestFeasibleInsertion(req, s.routes[k]);

          if (r.cost != MAX_FLOAT) {
            inserted = true;
            at = r;
          }
        }
      }

      if (inserted) {
        insertedUsers++;
        s.routes.at(at.vehicle.id - 1) = at;
      }
    }
  }

  if (insertedUsers == c) {
    std::cout << "\nLess vehicles" << '\n';
    s.routes.erase(s.routes.begin());
  }
  else {
    s = original;
  }

  s.computeCost(penaltyParams);

  return s;
}

Solution Grasp::_2opt(Solution s, std::vector<float> penaltyParams)
{
  for (int k = 0; k < s.routes.size(); k++) {

    start:
    for (int a = 1; a < s.routes[k].path.size() - 3; a++) {
      for (int b = a + 2; b < s.routes[k].path.size() - 2; b++) {
        for (int c = b + 2; c < s.routes[k].path.size() - 1; c++) {
          Route r = s.routes[k];

          Node *A = r.path[a - 1];
          Node *B = r.path[a];
          Node *C = r.path[b - 1];
          Node *D = r.path[b];
          Node *E = r.path[c - 1];
          Node *F = r.path[c % s.routes[k].path.size()];

          float d0 = instance->getTravelTime(A,B) + instance->getTravelTime(C,D) + instance->getTravelTime(E,F);
          float d1 = instance->getTravelTime(A,C) + instance->getTravelTime(B,D) + instance->getTravelTime(E,F);
          float d2 = instance->getTravelTime(A,B) + instance->getTravelTime(C,E) + instance->getTravelTime(D,F);
          float d3 = instance->getTravelTime(A,D) + instance->getTravelTime(E,B) + instance->getTravelTime(C,F);
          float d4 = instance->getTravelTime(F,B) + instance->getTravelTime(C,D) + instance->getTravelTime(E,A);

          if (d1 < d0) {
            std::reverse(r.path.begin() + a, r.path.begin() + b + 1);
            r.performEightStepEvaluationScheme();
          }
          else if (d2 < d0) {
            std::reverse(r.path.begin() + b, r.path.begin() + c + 1);
            r.performEightStepEvaluationScheme();
          }
          else if (d4 < d0) {
            std::reverse(r.path.begin() + a, r.path.begin() + c + 1);
            r.performEightStepEvaluationScheme();
          }
          else if (d3 < d0) {
            std::vector<Node *> temp;

            for (int rs = b; rs <= c; rs++)
              temp.push_back(r.path[rs]);

            for (int rs = a; rs <= b; rs++)
              temp.push_back(r.path[rs]);

            for (int rs = a; rs <= c; rs++)
              r.path[rs] = temp[rs - a];

            r.performEightStepEvaluationScheme();
          }

          bool valid = true;

          for (int m = 0; m < r.path.size(); m++) {
            if (r.path[m]->isPickup() && r.getDeliveryIndexOf(m) < m)
              valid = false;
          }

          if (valid && r.isFeasible() && r.cost < s.routes[k].cost) {
            s.routes[k] = r;
            goto start;
          }
        }
      }
    }
  }

  // for (int k = 0; k < s.routes.size(); k++) {
  //   start:
  //   for (int i = 1; i < s.routes[k].path.size() - 2; i++) {
  //     for (int j = i + 1; j < s.routes[k].path.size() - 1; j++) {
  //       Route r = s.routes[k];
  //       std::reverse(r.path.begin() + i, r.path.begin() + j + 1);
  //       r.performEightStepEvaluationScheme();

        // bool valid = true;

        // for (int m = 0; m < r.path.size(); m++) {
        //   if (r.path[m]->isPickup() && r.getDeliveryIndexOf(m) < m)
        //     valid = false;
        // }

  //       if (valid && r.isFeasible() && r.cost < s.routes[k].cost) {
  //         s.routes[k] = r;
  //         goto start;
  //       }
  //     }
  //   }
  // }

  s.computeCost(penaltyParams);
  // for (int k = 0; k < s.routes.size(); k++) {
  //   Route r = s.routes[k];

  //   for (int i = 1; i < r.path.size() - 2; i++) {
  //     for (int j = i + 1; j < r.path.size() - 1; j++) {
  //       std::reverse(r.path.begin() + i, r.path.begin() + j + 1);
  //       r.performEightStepEvaluationScheme();

  //       bool valid = true;

  //       for (int m = 0; m < r.path.size(); m++) {
  //         if (r.path[m]->isPickup())
  //           if (r.getDeliveryIndexOf(m) < m)
  //             valid = false;
  //       }

  //       if (valid && r.isFeasible() && r.cost < best.routes[k].cost) {
  //         best.routes[k] = r;
  //         best.computeCost(penaltyParams);
  //       }
  //       else {
  //         r = best.routes[k];
  //       }
  //     }
  //   }
  // }

  return s;
}

Solution Grasp::swapZeroOne(Solution s, std::vector<float> penaltyParams)
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

            if (r1.isFeasible() && r2.cost != MAX_FLOAT && r1.cost + r2.cost < best.routes[k1].cost + best.routes[k2].cost) {
              best = s;
              best.routes[k1] = r1;
              best.routes[k2] = r2;
            }
          }
        }
      }
    }
  }

  best.computeCost(penaltyParams);

  return best;
}

Solution Grasp::relocate(Solution s, std::vector<float> penaltyParams)
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

        if (r.cost != MAX_FLOAT && r.cost < best.routes[k].cost) {
          best.routes[k] = r;
        }
      }
    }
  }

  best.computeCost(penaltyParams);

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
  double rand = Prng::generateFloatInRange(0, 1);
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

void Grasp::adjustPenaltyParams(Solution s, std::vector<float> &penaltyParams, float delta)
{
  float factor = 1 + delta;

  s.loadViolation         == 0 ? penaltyParams[0] /= factor : penaltyParams[0] *= factor;
  s.timeWindowViolation   == 0 ? penaltyParams[1] /= factor : penaltyParams[1] *= factor;
  s.maxRideTimeViolation  == 0 ? penaltyParams[2] /= factor : penaltyParams[2] *= factor;
  s.batteryLevelViolation == 0 ? penaltyParams[3] /= factor : penaltyParams[3] *= factor;
  s.finalBatteryViolation == 0 ? penaltyParams[4] /= factor : penaltyParams[4] *= factor;
}

Route Grasp::performCheapestFeasibleInsertion(Request req, Route r)
{
  // Best insertion starts with infinity cost, we will update it during the search
  Route best = r;
  best.cost = MAX_FLOAT;

  for (int p = 1; p < r.path.size(); p++) {
    r.path.insert(r.path.begin() + p, req.pickup);

    for (int d = p + 1; d < r.path.size(); d++) {
      r.path.insert(r.path.begin() + d, req.delivery);
      r.performEightStepEvaluationScheme();

      // for (int b = 0; b < r.path.size(); b++) {
      //   if (r.batteryLevels[b] < 0 || (b == r.path.size() - 1 && r.batteryLevels[b] < r.vehicle.batteryCapacity * r.vehicle.minFinalBatteryRatioLevel)) {
      //     std::vector<Stop> stops;

      //     for (int s = 0; s < b; s++) {
      //       if (r.load[s] == 0 && s != r.path.size() - 1) {
      //         Stop stop;
      //         stop.position = s + 1;
      //         stop.station = instance->getNode(instance->nearestStations[r.path[s]->id][r.path[s + 1]->id]);
      //         stops.push_back(stop);
      //       }
      //     }

      //     for (Stop &s : stops) {
      //       r.path.insert(r.path.begin() + s.position, s.station);
      //       r.cost = performEightStepEvaluationScheme(r);

      //       if (r.isFeasible() && r.cost < best.cost) {
      //         std::vector<Stop> bestStops;
      //         bestStops.push_back(s);
      //         best = {r, r.cost, p, d, bestStops};
      //       }

      //       r.path.erase(r.path.begin() + s.position);
      //     }
      //   }
      // }

      if (r.isFeasible() && r.cost < best.cost)
        best = r;

      r.path.erase(r.path.begin() + d);
    }

    r.path.erase(r.path.begin() + p);
  }

  return best;
}
