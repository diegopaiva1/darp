/**
 * @file    Grasp.hpp
 * @author  Diego Paiva
 * @date    26/09/2019
 */

#ifndef GRASP_H_INCLUDED
#define GRASP_H_INCLUDED

#include "../data-structures/Singleton.hpp"
#include "../data-structures/Solution.hpp"
#include "../utils/Prng.hpp"

class Grasp
{
public:
  static Solution solve()
  {
    float alpha = 1.0, beta = 1.0, gama = 1.0, delta = Prng::generateFloatInRange(0.05, 0.10);
    Solution solution;
    std::vector<Request*> requests;

    for (Request *r : Singleton::getInstance()->requests)
      requests.push_back(r);

    for (Request *r : requests)
      printf("(%d, %d)", r->pickup->id, r->delivery->id);

    for (Vehicle *v : Singleton::getInstance()->vehicles)
      solution.routes.push_back(new Route(v));

    std::sort(requests.begin(), requests.end(), [](Request *&r1, Request *&r2) {
      return r1->getTimeWindowMedian() < r2->getTimeWindowMedian();
    });

    // std::random_shuffle(requests.begin(), requests.end(), [](int i) { return rand() % i; });

    for (Route *route : solution.routes) {
      Request *request = requests[0];

      route->path.push_back(Singleton::getInstance()->getOriginDepot());
      route->path.push_back(Singleton::getInstance()->getDestinationDepot());
      route->path.insert(route->path.begin() + 1, request->pickup);
      route->path.insert(route->path.begin() + 2, request->delivery);

      requests.erase(requests.begin());
    }

    while (!requests.empty()) {
      Request *request = requests[0];
      performCheapestFeasibleInsertion(request, solution, alpha, beta, gama, delta);
      requests.erase(requests.begin());
    }

    float f = performEightStepEvaluationScheme(solution, alpha, beta, gama, delta);

    return solution;
  }

  static float performEightStepEvaluationScheme(Solution &solution, float &alpha, float &beta, float &gama, float delta)
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
        computeLoad(r, i);
        computeArrivalTime(r, i);
        computeServiceBeginningTime(r, i);
        computeWaitingTime(r, i);
        computeDepartureTime(r, i);
      }

      for (int i = 0; i < r->path.size(); i++) {
        // Detected load violation or time window violation thus solution is infeasible
        if (r->load[i] > r->vehicle->capacity || r->serviceBeginningTimes[i] > r->path[i]->departureTime)
          return violations(solution, alpha, beta, gama, delta);
      }

      // ===================== STEP 3 =====================
      float f0 = calculateForwardSlackTime(0, r);

      // ===================== STEP 4 =====================
      float waitingTimeSum = 0.0;
      for (int i = 1; i < r->path.size() - 1; i++)
        waitingTimeSum += r->waitingTimes[i];

      r->departureTimes[0] = r->path[0]->arrivalTime + std::min(f0, waitingTimeSum);

      // ===================== STEP 5 =====================
      for (int i = 0; i < r->path.size(); i++) {
        computeArrivalTime(r, i);
        computeServiceBeginningTime(r, i);
        computeWaitingTime(r, i);
        computeDepartureTime(r, i);
      }

      // ===================== STEP 6 =====================
      bool allRidingTimesRespected = true;

      for (int i = 1; i < r->path.size() - 1; i++) {
        if (r->path[i]->type == Type::PICKUP) {
          computeRidingTime(r, i);

          if (r->ridingTimes[i] > r->path[i]->maxRideTime)
            allRidingTimesRespected = false;
        }
      }

      // This solution is feasible
      if (allRidingTimesRespected)
        return violations(solution, alpha, beta, gama, delta);

      // ===================== STEP 7 =====================
      for (int i = 1; i < r->path.size() - 1; i++) {
        if (r->path[i]->type == Type::PICKUP) {
          // ===================== STEP 7.a =====================
          float forwardSlackTime = calculateForwardSlackTime(i, r);

          // ===================== STEP 7.b =====================
          float waitingTimeSum = 0.0;
          for (int p = i + 1; p < r->path.size() - 1; p++)
            waitingTimeSum += r->waitingTimes[p];

          r->waitingTimes[i] += std::min(forwardSlackTime, waitingTimeSum);
          r->serviceBeginningTimes[i] = r->arrivalTimes[i] + r->waitingTimes[i];
          r->departureTimes[i] = r->serviceBeginningTimes[i] + r->path[i]->serviceTime;

          // ===================== STEP 7.c =====================
          for (int j = i + 1; j < r->path.size(); j++) {
            computeArrivalTime(r, j);
            computeServiceBeginningTime(r, j);
            computeWaitingTime(r, j);
            computeDepartureTime(r, j);
          }

          // ===================== STEP 7.d =====================
          allRidingTimesRespected = true;

          for (int k = i + 1; k < r->path.size() - 1; k++) {
            if (r->path[k]->type == Type::DELIVERY) {
              int pickupIndex = getPickupIndexOf(r, k);

              computeRidingTime(r, pickupIndex);

              if (r->ridingTimes[pickupIndex] > r->path[pickupIndex]->maxRideTime)
                allRidingTimesRespected = false;
            }
          }

          // This solution is feasible
          if (allRidingTimesRespected)
            return violations(solution, alpha, beta, gama, delta);
        }
      }
    }

    return violations(solution, alpha, beta, gama, delta);
  }

  static float violations(Solution s, float &alpha, float &beta, float &gama, float delta)
  {
    float routeCost            = 0.0;
    float timeWindowViolation  = 0.0;
    float maxRideTimeViolation = 0.0;
    int   loadViolation        = 0.0;

    for (Route *r : s.routes) {
      for (int i = 0; i < r->path.size(); i++) {
        Node *node = r->path[i];

        if (i != r->path.size() - 1)
          routeCost += r->path[i]->point->getDistanceFrom(r->path[i + 1]->point);

        if (node->type == Type::PICKUP)
          maxRideTimeViolation += std::max(0.0f, r->ridingTimes[i] - node->maxRideTime);

        if (node->type == Type::PICKUP || node->type == Type::DELIVERY) {
          loadViolation       += std::max(0, r->load[i] - r->vehicle->capacity);
          timeWindowViolation += std::max(0.0f, r->serviceBeginningTimes[i] - node->departureTime);
        }

        float factor = 1 + delta;

        // Ajusta os parâmetros de penalidade para cada restrição
        loadViolation        == 0 ? alpha /= factor : alpha *= factor;
        timeWindowViolation  == 0 ? beta  /= factor : beta  *= factor;
        maxRideTimeViolation == 0 ? gama  /= factor : gama  *= factor;
      }
    }

    return routeCost + alpha * loadViolation + beta * timeWindowViolation + gama * maxRideTimeViolation;
  }

  /* O forward slack time é calculado como o menor de todos os slack times entre
   * o index (ponto da rota que se deseja calcular) e o ponto final da rota
   */
  static float calculateForwardSlackTime(int index, Route *r)
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
        if (r->path[i]->id == j - Singleton::getInstance()->requestsAmount)
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


  static void performCheapestFeasibleInsertion(Request *&request, Solution &solution, float &alpha, float &beta, float &gama, float delta)
  {
    float bestCost = std::numeric_limits<float>::max();
    int routeId = -9999;
    int bestPickupIndex = -9999;
    int bestDeliveryIndex = -9999;

    for (int i = 0; i < solution.routes.size(); i++) {
      Route *r = solution.routes[i];

      for (int p = 1; p < r->path.size(); p++) {
        r->path.insert(r->path.begin() + p, request->pickup);

        for (int d = p + 1; d < r->path.size(); d++) {
          r->path.insert(r->path.begin() + d, request->delivery);

          float f = performEightStepEvaluationScheme(solution, alpha, beta, gama, delta);
          //std::cout << f << " = " << r->getTotalDistance() << "\n";

          if (f == r->getTotalDistance() && f < bestCost) {
            bestCost = f;
            routeId = i;
            bestPickupIndex = p;
            bestDeliveryIndex = d;
          }

          r->path.erase(r->path.begin() + d);
        }

        r->path.erase(r->path.begin() + p);
      }
    }

    if (routeId == -9999 && bestPickupIndex == -9999 && bestDeliveryIndex == -9999) {
      Route *route = new Route(new Vehicle(solution.routes.size() + 1));
      route->path.push_back(Singleton::getInstance()->getOriginDepot());
      route->path.push_back(request->pickup);
      route->path.push_back(request->delivery);
      route->path.push_back(Singleton::getInstance()->getDestinationDepot());
      solution.routes.push_back(route);
    }
    else {
      Route *best = solution.routes.at(routeId);
      best->path.insert(best->path.begin() + bestPickupIndex,   request->pickup);
      best->path.insert(best->path.begin() + bestDeliveryIndex, request->delivery);
    }
  }

  static void computeLoad(Route *&r, int i)
  {
    if (i == 0)
      r->load[i] = 0;
    else
      r->load[i] = r->load[i - 1] + r->path[i]->load;
  }

  static void computeArrivalTime(Route *&r, int i)
  {
    if (i == 0)
      r->arrivalTimes[i] = 0;
    else
      r->arrivalTimes[i] = r->departureTimes[i - 1] + Singleton::getInstance()->getTravelTime(r->path[i - 1], r->path[i]);
  }

  static void computeServiceBeginningTime(Route *&r, int i)
  {
    if (i == 0)
      r->serviceBeginningTimes[i] = r->departureTimes[i];
    else
      r->serviceBeginningTimes[i] = std::max(r->arrivalTimes[i], r->path[i]->arrivalTime);
  }

  static void computeWaitingTime(Route *&r, int i)
  {
    if (i == 0)
      r->waitingTimes[i] = 0;
    else
      r->waitingTimes[i] = r->serviceBeginningTimes[i] - r->arrivalTimes[i];
  }

  static void computeDepartureTime(Route *&r, int i)
  {
    if (i == r->path.size() - 1)
      r->departureTimes[i] = 0;
    else
      r->departureTimes[i] = r->serviceBeginningTimes[i] + r->path[i]->serviceTime;
  }

  static void computeRidingTime(Route *&r, int i)
  {
    r->ridingTimes[i] = r->serviceBeginningTimes[getDeliveryIndexOf(r, i)] - r->departureTimes[i];
  }

  // Retorna o índice 'i' de desembarque (delivery) de um nó 'j' de embarque (pickup) da rota
  static int getDeliveryIndexOf(Route *&r, int j)
  {
    if (r->path[j]->type != Type::PICKUP)
      throw "O nó fornecido não é um ponto de embarque";

    for (int i = 1; i < r->path.size(); i++)
      if (r->path[i]->id == r->path[j]->id + Singleton::getInstance()->requestsAmount)
        return i;
  }

  // Retorna o índice 'i' de embarque (pickup) de um nó 'j' de desembarque (delivery) da rota
  static int getPickupIndexOf(Route *&r, int j)
  {
    if (r->path[j]->type != Type::DELIVERY)
      throw "O nó fornecido não é um ponto de desembarque";

    for (int i = 1; i < r->path.size(); i++)
      if (r->path[i]->id == r->path[j]->id - Singleton::getInstance()->requestsAmount)
        return i;
  }
};

#endif // GRASP_H_INCLUDED
