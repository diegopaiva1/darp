/**
 * @file    Grasp.hpp
 * @author  Diego Paiva
 * @date    26/09/2019
 */

#ifndef GRASP_H_INCLUDED
#define GRASP_H_INCLUDED

#define ITERATIONS 1

#include "../data-structures/Singleton.hpp"
#include "../data-structures/Solution.hpp"
#include "../utils/Prng.hpp"

Singleton *instance = Singleton::getInstance();

class Grasp
{
public:
  static Solution solve()
  {
    Solution best;

    // In the beginning of the search, all penalty parameters are initialized with 1.0
    std::vector<float> penaltyParams = {1.0, 1.0, 1.0};

    // Random value helps adjusting the penalty parameters dinamically
    float delta = Prng::generateFloatInRange(0.05, 0.10);

    // GRASP's random factors vector
    std::vector<float> alphas = {0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50};

    for (int it = 1; it <= ITERATIONS; it++) {
      int index;
      int alphaIndex = Prng::generateIntegerInRange(0, alphas.size() - 1);
      Solution currSolution;
      std::vector<Request*> requests;

      for (Request *r : instance->requests)
        requests.push_back(r);

      for (Vehicle *v : instance->vehicles)
        currSolution.routes.push_back(new Route(v));

      std::sort(requests.begin(), requests.end(), [](Request *&r1, Request *&r2) {
        return r1->getTimeWindowMedian() < r2->getTimeWindowMedian();
      });

      for (Route *route : currSolution.routes) {
        it == 1 ? index = 0 : index = Prng::generateIntegerInRange(0, (int) (alphas[alphaIndex] * requests.size()));

        Request *request = requests[index];

        route->path.push_back(instance->getOriginDepot());
        route->path.push_back(instance->getDestinationDepot());
        route->path.insert(route->path.begin() + 1, request->pickup);
        route->path.insert(route->path.begin() + 2, request->delivery);

        requests.erase(requests.begin() + index);
      }

      while (!requests.empty()) {
        it == 1 ? index = 0 : index = Prng::generateIntegerInRange(0, (int) (alphas[alphaIndex] * requests.size()));
        Request *request = requests[index];
        performCheapestFeasibleInsertion(request, currSolution, penaltyParams, delta);
        requests.erase(requests.begin() + index);
      }

      for (Route *r : currSolution.routes)
        currSolution.cost += performEightStepEvaluationScheme(r, penaltyParams, delta);

     /* Everytime a new incumbent solution is found, we randomly choose a new delta. According to
      * (Parragh et. al, 2010) this works as a diversification mecanism and avoids cycling
      */
      delta = Prng::generateFloatInRange(0.05, 0.10);

      if ((it == 1) || (currSolution.routes.size() < best.routes.size()) ||
          (currSolution.routes.size() == best.routes.size() && currSolution.cost < best.cost))
        best = currSolution;

      printf("%d\n", it);

      std::cout << currSolution.cost << '\n';
    }

    return best;
  }

 /* The eight-step evaluation scheme is a procedure designed by (Cordeau and Laporte, 2003)
  * for the DARP which evaluates a given route in terms of cost and feasibility. This procedure
  * optimizes route duration and xcomplies with ride time constraint
  */
  static float performEightStepEvaluationScheme(Route *&r, std::vector<float> &penaltyParams, float &delta)
  {
    bool  allRidingTimesRespected;
    float forwardSlackTimeAtBeginning;
    float waitingTimeSum;

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
    r->chargingTimes.clear();
    r->chargingTimes.resize(size);

    STEP1:
      r->departureTimes[0] = r->serviceBeginningTimes[0];

    STEP2:
      for (int i = 0; i < r->path.size(); i++) {
        computeLoad(r, i);

        // Violated vehicle capacity, that's an irreparable violation
        if (r->load[i] > r->vehicle->capacity)
          goto STEP8;

        computeArrivalTime(r, i);
        computeServiceBeginningTime(r, i);

        // Violated time windows, that's an irreparable violation
        if (r->serviceBeginningTimes[i] > r->path[i]->departureTime)
          goto STEP8;

        computeWaitingTime(r, i);
        computeDepartureTime(r, i);
      }

    STEP3:
      forwardSlackTimeAtBeginning = calculateForwardSlackTime(0, r);

    STEP4:
      waitingTimeSum = 0.0;

      for (int i = 1; i < r->path.size(); i++)
        waitingTimeSum += r->waitingTimes[i];

      r->departureTimes[0] = r->path[0]->arrivalTime + std::min(forwardSlackTimeAtBeginning, waitingTimeSum);

    STEP5:
      for (int i = 0; i < r->path.size(); i++) {
        computeArrivalTime(r, i);
        computeServiceBeginningTime(r, i);
        computeWaitingTime(r, i);
        computeDepartureTime(r, i);
      }

    STEP6:
      allRidingTimesRespected = true;

      for (int i = 1; i < r->path.size() - 1; i++)
        if (r->path[i]->isPickup()) {
          computeRidingTime(r, i);

          if (r->ridingTimes[i] > r->path[i]->maxRideTime)
            allRidingTimesRespected = false;
        }

      if (allRidingTimesRespected)
        goto STEP8;

    STEP7:
      for (int i = 1; i < r->path.size() - 1; i++) {
        if (r->path[i]->isPickup()) {
          STEP7a:
            float forwardSlackTime = calculateForwardSlackTime(i, r);

          STEP7b:
            float waitingTimeSum = 0.0;

            for (int p = i + 1; p < r->path.size(); p++)
              waitingTimeSum += r->waitingTimes[p];

            r->waitingTimes[i] += std::min(forwardSlackTime, waitingTimeSum);
            r->serviceBeginningTimes[i] = r->arrivalTimes[i] + r->waitingTimes[i];
            r->departureTimes[i] = r->serviceBeginningTimes[i] + r->path[i]->serviceTime;

          STEP7c:
            for (int j = i + 1; j < r->path.size(); j++) {
              computeArrivalTime(r, j);
              computeServiceBeginningTime(r, j);
              computeWaitingTime(r, j);
              computeDepartureTime(r, j);
            }

          STEP7d:
            allRidingTimesRespected = true;

            for (int j = i + 1; j < r->path.size() - 1; j++)
              if (r->path[j]->isDelivery()) {
                int pickupIndex = getPickupIndexOf(r, j);
                computeRidingTime(r, pickupIndex);

                if (r->ridingTimes[pickupIndex] > r->path[pickupIndex]->maxRideTime)
                  allRidingTimesRespected = false;
              }

            if (allRidingTimesRespected)
              goto STEP8;
        }
      }

    STEP8:
      r->cost                 = 0.0;
      r->loadViolation        = 0;
      r->timeWindowViolation  = 0.0;
      r->maxRideTimeViolation = 0.0;

      for (int i = 0; i < r->path.size(); i++) {
        if (i > 0)
          r->cost += instance->getTravelTime(r->path[i], r->path[i - 1]);

        if (r->path[i]->isPickup())
          r->maxRideTimeViolation += std::max(0.0f, r->ridingTimes[i] - r->path[i]->maxRideTime);

        r->loadViolation       += std::max(0, r->load[i] - r->vehicle->capacity);
        r->timeWindowViolation += std::max(0.0f, r->serviceBeginningTimes[i] - r->path[i]->departureTime);
      }

      // In below code, we adjust penalty parameters according to the violations
      float factor = 1 + delta;

      r->loadViolation        == 0 ? penaltyParams[0] /= factor : penaltyParams[0] *= factor;
      r->timeWindowViolation  == 0 ? penaltyParams[1] /= factor : penaltyParams[1] *= factor;
      r->maxRideTimeViolation == 0 ? penaltyParams[2] /= factor : penaltyParams[2] *= factor;

      return r->cost + r->loadViolation        * penaltyParams[0] +
                       r->timeWindowViolation  * penaltyParams[1] +
                       r->maxRideTimeViolation * penaltyParams[2];
  }

  /* O forward slack time é calculado como o menor de todos os slack times entre
   * o index (ponto da rota que se deseja calcular) e o ponto final da rota
   */
  static float calculateForwardSlackTime(int index, Route *r)
  {
    float forwardSlackTime;

    for (int j = index; j < r->path.size(); j++) {
      float waitingTimeSum = 0.0;

      for (int p = index + 1; p <= j; p++)
        waitingTimeSum += r->waitingTimes[p];

      float userRideTimeWithDeliveryAtJ = 0.0;
      bool jMinusNIsVisitedBeforeIndex  = false;

      for (int p = 0; p <= index; p++)
        if (r->path[p]->id == j - instance->requestsAmount < index)
          jMinusNIsVisitedBeforeIndex = true;

      if (r->path[j]->isDelivery() && jMinusNIsVisitedBeforeIndex)
        userRideTimeWithDeliveryAtJ = r->ridingTimes[getPickupIndexOf(r, j)];

      float slackTime;

      if (r->path[index]->isPickup())
        slackTime = waitingTimeSum +
                    std::max(0.0f, std::min(r->path[j]->departureTime - r->serviceBeginningTimes[j],
                                            r->path[index]->maxRideTime - userRideTimeWithDeliveryAtJ));
      else if (index == 0)
        slackTime = waitingTimeSum + std::max(0.0f, r->path[j]->departureTime - r->serviceBeginningTimes[j]);
      else
        slackTime = 0.0;

      if (j == index || slackTime < forwardSlackTime)
        forwardSlackTime = slackTime;
    }

    return forwardSlackTime;
  }


  static void performCheapestFeasibleInsertion(Request *&request, Solution &solution, std::vector<float> &penaltyParams, float &delta)
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

          r->cost = performEightStepEvaluationScheme(r, penaltyParams, delta);

          if (r->isFeasible() && r->cost < bestCost) {
            bestCost = r->cost;
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
      route->path.push_back(instance->getOriginDepot());
      route->path.push_back(request->pickup);
      route->path.push_back(request->delivery);
      route->path.push_back(instance->getDestinationDepot());
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
      r->arrivalTimes[i] = r->departureTimes[i - 1] + instance->getTravelTime(r->path[i - 1], r->path[i]);
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
    // else if (r->path[i]->isStation())
    //   r->departureTimes[i] = r->serviceBeginningTimes[i] + (r->batteryLevels[i] - r->batteryLevels[i - 1])/
    //   r->path[i]->rechargingRate;
    else
      r->departureTimes[i] = r->serviceBeginningTimes[i] + r->path[i]->serviceTime;
  }

  static void computeRidingTime(Route *&r, int i)
  {
    r->ridingTimes[i] = r->serviceBeginningTimes[getDeliveryIndexOf(r, i)] - r->departureTimes[i];
  }

  static void computeChargingTime(Route *&r, int i)
  {
    if (r->path[i]->isStation())
      r->chargingTimes[i] = r->serviceBeginningTimes[i] - r->serviceBeginningTimes[i - 1] -
                            instance->getTravelTime(r->path[i - 1], r->path[i]);
    else
      r->chargingTimes[i] = 0.0;
  }

  // TODO: recarga parcial...
  static void computeBatteryLevel(Route *&r, int i)
  {
    // if (r->path[i]->isStation() || r->path[i]->isDepot())
    //   r->batteryLevels[i] = r->vehicle->batteryCapacity/5;
    // else
    //   r->batteryLevels[i] = r->batteryLevels[i - 1] -
    //                         r->vehicle->dischargingRate * instance->getTravelTime(r->path[i - 1], r->path[i]);
    if (r->path[i]->isStation() || r->path[i]->isDepot())
      r->batteryLevels[i] = r->vehicle->batteryCapacity/5;
    else
      r->batteryLevels[i] = r->batteryLevels[i - 1] -
                            r->vehicle->dischargingRate * instance->getTravelTime(r->path[i - 1], r->path[i]);
  }

  // Retorna o índice 'i' de desembarque (delivery) de um nó 'j' de embarque (pickup) da rota
  static int getDeliveryIndexOf(Route *&r, int j)
  {
    if (r->path[j]->type != Type::PICKUP)
      throw "O nó fornecido não é um ponto de embarque";

    for (int i = 1; i < r->path.size(); i++)
      if (r->path[i]->id == r->path[j]->id + instance->requestsAmount)
        return i;
  }

  // Retorna o índice 'i' de embarque (pickup) de um nó 'j' de desembarque (delivery) da rota
  static int getPickupIndexOf(Route *&r, int j)
  {
    if (r->path[j]->type != Type::DELIVERY)
      throw "O nó fornecido não é um ponto de desembarque";

    for (int i = 1; i < r->path.size(); i++)
      if (r->path[i]->id == r->path[j]->id - instance->requestsAmount)
        return i;
  }
};

#endif // GRASP_H_INCLUDED
