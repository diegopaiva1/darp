/**
 * @file    InsertionHeuristic.hpp
 * @author  Diego Paiva
 * @date    26/09/2019
 */

#ifndef INSERTIONHEURISTIC_H_INCLUDED
#define INSERTIONHEURISTIC_H_INCLUDED

#include "../data-structures/Instance.hpp"
#include "../data-structures/Solution.hpp"
#include "../utils/Prng.hpp"

class InsertionHeuristic
{
public:
  static Solution getSolution(Instance instance)
  {
    Solution solution;
    std::vector<Request*> requests;

    for (Request *r : instance.requests)
      requests.push_back(r);

    for (Vehicle *v : instance.vehicles)
      solution.routes.push_back(new Route(v));

    std::sort(requests.begin(), requests.end(), [](Request *&r1, Request *&r2) {
      return r1->getTimeWindowMedian() < r2->getTimeWindowMedian();
    });

    for (Route *route : solution.routes) {
      route->path.push_back(instance.getOriginDepot());
      route->path.push_back(instance.getDestinationDepot());
    }

    while (!requests.empty()) {
      Request *request = requests[0];
      performCheapestInsertion(request, solution);
      requests.erase(requests.begin());
    }

    float f = performEightStepEvaluationScheme(solution, instance);
    printf("f(s) = %f e c(s) = %f", f, solution.cost());
    return solution;
  }

  static float performEightStepEvaluationScheme(Solution solution, Instance instance)
  {
    for (Route *&r : solution.routes) {
      int size = r->path.size();
      r->arrivalTimes.clear();
      r->arrivalTimes.resize(size);
      r->serviceBeginningTimes.clear();
      r->serviceBeginningTimes.resize(size);
      r->departureTimes.clear();
      r->departureTimes.resize(size);
      r->waitingTimes.clear();
      r->waitingTimes.resize(size);
      r->ridingTimes.clear();
      r->ridingTimes.resize(size);
      r->load.clear();
      r->load.resize(size);
      r->batteryLevels.clear();
      r->batteryLevels.resize(size);
    }

    for (Route *&r : solution.routes) {
      // ================== STEP 1 ==================
      r->departureTimes[0] = r->path[0]->arrivalTime;

      // ================== STEP 2 ==================
      for (int i = 0; i < r->path.size(); i++) {
        i == 0 ? r->load[i] = 0
               : r->load[i] = r->load[i - 1] + r->path[i]->load;

        // Detected load violation, this solution is infeasible
        if (r->load[i] > r->vehicle->capacity)
          return violations(solution);

        i == 0 ? r->arrivalTimes[i] = 0
               : r->arrivalTimes[i] = r->departureTimes[i - 1] + instance.getTravelTime(r->path[i - 1], r->path[i]);

        i == 0 ? r->serviceBeginningTimes[i] = r->departureTimes[i]
               : r->serviceBeginningTimes[i] = std::max(r->arrivalTimes[i], r->path[i]->arrivalTime);

        // Detected time window violation, this solution is infeasible
        if (r->serviceBeginningTimes[i] > r->path[i]->departureTime)
          return violations(solution);

        i == 0 ? r->waitingTimes[i] = 0
               : r->waitingTimes[i] = r->serviceBeginningTimes[i] - r->arrivalTimes[i];

        i == r->path.size() - 1 ? r->departureTimes[i] = 0
                                : r->departureTimes[i] = r->serviceBeginningTimes[i] + r->path[i]->serviceTime;
      }

      // ===================== STEP 3 =====================
      float f0 = calculateForwardSlackTime(0, r, instance);

      // ===================== STEP 4 =====================
      float waitingTimeSum = 0.0;
      for (int i = 1; i < r->path.size() - 1; i++)
        waitingTimeSum += r->waitingTimes[i];

      r->departureTimes[0] = r->path[0]->arrivalTime + std::min(f0, waitingTimeSum);

      // ===================== STEP 5 =====================
      for (int i = 0; i < r->path.size(); i++) {
        i == 0 ? r->arrivalTimes[i] = 0
               : r->arrivalTimes[i] = r->departureTimes[i - 1] + instance.getTravelTime(r->path[i - 1], r->path[i]);

        i == 0 ? r->serviceBeginningTimes[i] = r->departureTimes[i]
               : r->serviceBeginningTimes[i] = std::max(r->arrivalTimes[i], r->path[i]->arrivalTime);

        i == 0 ? r->waitingTimes[i] = 0
               : r->waitingTimes[i] = r->serviceBeginningTimes[i] - r->arrivalTimes[i];

        i == r->path.size() - 1 ? r->departureTimes[i] = 0
                                : r->departureTimes[i] = r->serviceBeginningTimes[i] + r->path[i]->serviceTime;
      }

      // ===================== STEP 6 =====================
      bool allRidingTimesRespected = true;

      for (int i = 1; i < r->path.size() - 1; i++) {
        if (r->path[i]->type == Type::PICKUP) {
          int did;

          for (int j = 1; j < r->path.size() - 1; i++)
            if (r->path[j]->id == i + instance.requestsAmount)
              did = j;

          r->ridingTimes[i] = r->serviceBeginningTimes[did] - r->departureTimes[i];

          if (r->ridingTimes[i] > r->path[i]->maxRideTime)
            allRidingTimesRespected = false;
        }
      }

      // This solution is feasible
      if (allRidingTimesRespected)
        return violations(solution);

      // ===================== STEP 7 =====================
      for (int i = 1; i < r->path.size() - 1; i++) {
        if (r->path[i]->type == Type::PICKUP) {
          // ===================== STEP 7.a =====================
          float forwardSlackTime = calculateForwardSlackTime(i, r, instance);

          // ===================== STEP 7.b =====================
          float waitingTimeSum = 0.0;
          for (int p = i + 1; p < r->path.size() - 1; p++)
            waitingTimeSum += r->waitingTimes[p];

          r->waitingTimes[i] += std::min(forwardSlackTime, waitingTimeSum);
          r->serviceBeginningTimes[i] = r->arrivalTimes[i] + r->waitingTimes[i];
          r->departureTimes[i] = r->serviceBeginningTimes[i] + r->path[i]->serviceTime;

          // ===================== STEP 7.c =====================
          for (int j = i + 1; j < r->path.size(); j++) {
            i == 0 ? r->arrivalTimes[i] = 0
                   : r->arrivalTimes[i] = r->departureTimes[i - 1] + instance.getTravelTime(r->path[i - 1], r->path[i]);

            i == 0 ? r->serviceBeginningTimes[i] = r->departureTimes[i]
                   : r->serviceBeginningTimes[i] = std::max(r->arrivalTimes[i], r->path[i]->arrivalTime);

            i == 0 ? r->waitingTimes[i] = 0
                   : r->waitingTimes[i] = r->serviceBeginningTimes[i] - r->arrivalTimes[i];

            i == r->path.size() - 1 ? r->departureTimes[i] = 0
                                    : r->departureTimes[i] = r->serviceBeginningTimes[i] + r->path[i]->serviceTime;
          }

          // ===================== STEP 7.d =====================
          allRidingTimesRespected = true;

          for (int k = i + 1; k < r->path.size() - 1; k++) {
            if (r->path[k]->type == Type::DELIVERY) {
              int pid;

              for (int req = 1; req < r->path.size() - 1; req++)
                if (r->path[req]->id == k - instance.requestsAmount)
                  pid = req;

              r->ridingTimes[pid] = r->serviceBeginningTimes[k] - r->departureTimes[pid];

              if (r->ridingTimes[pid] > r->path[pid]->maxRideTime)
                allRidingTimesRespected = false;
            }
          }

          // This solution is feasible
          if (allRidingTimesRespected)
            return violations(solution);
        }
      }
    }

    return violations(solution);
  }

  static float violations(Solution s)
  {
    float alpha = 1.0;
    float beta  = 1.0;
    float gama  = 1.0;

    float timeWindowViolation  = 0.0;
    float maxRideTimeViolation = 0.0;
    float loadViolation        = 0.0;

    for (Route *r : s.routes) {
      for (int i = 0; i < r->path.size(); i++) {
        Node *node = r->path[i];

        if (node->type == Type::PICKUP)
          maxRideTimeViolation += std::max(0.0f, r->ridingTimes[i] - node->maxRideTime);

        if (node->type == Type::PICKUP || node->type == Type::DELIVERY) {
          loadViolation       += std::max(0, r->load[i] - r->vehicle->capacity);
          timeWindowViolation += std::max(0.0f, r->serviceBeginningTimes[i] - node->departureTime);
        }

        // float factor = 1 + delta;

        // // Ajusta os parâmetros de penalidade para cada restrição
        // loadViolation        == 0 ? alpha /= factor : alpha *= factor;
        // timeWindowViolation  == 0 ? beta  /= factor : beta  *= factor;
        // maxRideTimeViolation == 0 ? gama   /= factor : gama   *= factor;
      }
    }

    return s.cost() + alpha * timeWindowViolation + beta * maxRideTimeViolation + gama * loadViolation;
  }

  /* O forward slack time é calculado como o menor de todos os slack times entre
   * o index (ponto da rota que se deseja calcular) e o ponto final da rota
   */
  static float calculateForwardSlackTime(int index, Route *r, Instance instance)
  {
    float forwardSlackTime;

    for (int j = index; j < r->path.size(); j++) {
      int pid;
      float slackTime;
      float waitingTimeSum = 0.0;
      float userRideTimeWithDeliveryAtJ = 0.0;

      for (int p = index + 1; p <= j; p++)
        waitingTimeSum += r->waitingTimes[p];

      for (int i = 1; i < r->path.size() - 1; i++)
        if (r->path[i]->id == j - instance.requestsAmount)
          pid = i;

      if (r->path[j]->type == Type::DELIVERY && pid < index)
        userRideTimeWithDeliveryAtJ = r->ridingTimes[pid];

      if (r->path[index]->type == Type::PICKUP)
        slackTime = waitingTimeSum +
                    std::max(0.0f, std::min(r->path[j]->arrivalTime     - r->serviceBeginningTimes[j],
                                            r->path[index]->maxRideTime - userRideTimeWithDeliveryAtJ));
      else if (index == 0)
        slackTime = waitingTimeSum + std::max(0.0f, r->path[j]->arrivalTime - r->serviceBeginningTimes[j]);
      else
        slackTime = 0.0;

      if (j == index || slackTime < forwardSlackTime)
        forwardSlackTime = slackTime;
    }

    return forwardSlackTime;
  }


  static void performCheapestInsertion(Request *&request, Solution &solution)
  {
    float bestCost = std::numeric_limits<float>::max();
    int routeId;
    int bestPickupIndex;
    int bestDeliveryIndex;

    for (int i = 0; i < solution.routes.size(); i++) {
      Route *r = solution.routes[i];

      for (int p = 1; p < r->path.size(); p++) {
        r->path.insert(r->path.begin() + p, request->pickup);

        for (int d = p + 1; d < r->path.size(); d++) {
          r->path.insert(r->path.begin() + d, request->delivery);

          if (r->getTotalDistance() < bestCost) {
            bestCost = r->getTotalDistance();
            routeId = i;
            bestPickupIndex = p;
            bestDeliveryIndex = d;
          }

          r->path.erase(r->path.begin() + d);
        }

        r->path.erase(r->path.begin() + p);
      }
    }

    Route *best = solution.routes.at(routeId);
    best->path.insert(best->path.begin() + bestPickupIndex,   request->pickup);
    best->path.insert(best->path.begin() + bestDeliveryIndex, request->delivery);
  }
};

#endif // INSERTIONHEURISTIC_H_INCLUDED
