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

Singleton *instance = Singleton::getInstance();

class Grasp
{
public:
  static Solution solve()
  {
    Solution best;
    std::vector<float> alphas = {0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50};

    for (int it = 1; it <= 1; it++) {
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
        performCheapestFeasibleInsertion(request, currSolution);
        requests.erase(requests.begin() + index);
      }

      for (Route *r : currSolution.routes)
        performEightStepEvaluationScheme(r);

      if ((it == 1) || (currSolution.routes.size() < best.routes.size()) ||
          (currSolution.routes.size() == best.routes.size() && currSolution.cost() < best.cost()))
        best = currSolution;

      std::cout << currSolution.cost() << '\n';
    }

    return best;
  }

  static void performEightStepEvaluationScheme(Route *&r)
  {
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

    // STEP 1: B_0 = e_0; D_0 = B_0
    r->serviceBeginningTimes[0] = r->path[0]->arrivalTime;
    r->departureTimes[0] = r->serviceBeginningTimes[0];
    r->batteryLevels[0] = r->vehicle->initialBatteryLevel/5;

    // STEP 2: Compute A_i, W_i, B_i, D_i and Q_i for each vertex i in the route
    for (int i = 1; i < r->path.size(); i++) {
      computeLoad(r, i);

      // Violated vehicle capacity, that's an irreparable violation
      if (r->load[i] > r->vehicle->capacity) return;

      computeArrivalTime(r, i);
      computeServiceBeginningTime(r, i);

      // Violated time windows, that's an irreparable violation
      if (r->serviceBeginningTimes[i] > r->path[i]->departureTime) return;

      computeWaitingTime(r, i);
      computeDepartureTime(r, i);
      computeBatteryLevel(r, i);
      computeChargingTimes(r, i);
    }

    // STEP 3: Compute F_0
    float f0 = calculateForwardSlackTime(0, r);

    // STEP 4: B_0 = e_0 + min{f_0, sum(W_p)}; D_0 = B_0
    float waitingTimeSum = 0.0;

    for (int i = 1; i < r->path.size(); i++)
      waitingTimeSum += r->waitingTimes[i];

    r->serviceBeginningTimes[0] = r->path[0]->arrivalTime + std::min(f0, waitingTimeSum);
    r->departureTimes[0] = r->serviceBeginningTimes[0];

    // STEP 5: Update A_i, W_i, B_i and D_i for each vertex i in the route
    for (int i = 1; i < r->path.size(); i++) {
      computeArrivalTime(r, i);
      computeServiceBeginningTime(r, i);
      computeWaitingTime(r, i);
      computeDepartureTime(r, i);
    }

    // STEP 6: Compute R_i for each request in the route
    for (int i = 1; i < r->path.size() - 1; i++)
      if (r->path[i]->isPickup())
        computeRidingTime(r, i);

    // STEP 7: For every vertex i that is an origin
    for (int i = 1; i < r->path.size() - 1; i++) {
      if (r->path[i]->isPickup()) {
        // STEP 7.a - Compute F_i
        float forwardSlackTime = calculateForwardSlackTime(i, r);

        // STEP 7.b - Set W_i = W_i + min{F_i, sum(W_p)}; B_i = A_i + W_i; D_i = B_i + d_i
        float waitingTimeSum = 0.0;
        for (int p = i + 1; p < r->path.size(); p++)
          waitingTimeSum += r->waitingTimes[p];

        r->serviceBeginningTimes[i] += std::min(forwardSlackTime, waitingTimeSum);
        r->departureTimes[i] = r->serviceBeginningTimes[i] + r->path[i]->serviceTime;
        r->waitingTimes[i] = r->serviceBeginningTimes[i] - r->arrivalTimes[i];

        // STEP 7.c - Update A_j, W_j, B_j and D_j for each vertex j that comes after i in the route
        for (int j = i + 1; j < r->path.size(); j++) {
          computeArrivalTime(r, j);
          computeServiceBeginningTime(r, j);
          computeWaitingTime(r, j);
          computeDepartureTime(r, j);
        }

        // STEP 7.d - Update R_j for each request j whose destination is after i
        for (int j = i + 1; j < r->path.size() - 1; j++) {
          if (r->path[j]->isDelivery()) {
            int pickupIndex = getPickupIndexOf(r, j);
            computeRidingTime(r, pickupIndex);
          }
        }
      }
    }
  }

  static bool isFeasible(Route *r)
  {
    for (int i = 0; i < r->path.size(); i++) {
      if (r->serviceBeginningTimes[i] > r->path[i]->departureTime)
        return false;

      if (r->path[i]->isPickup() && r->ridingTimes[i] > r->path[i]->maxRideTime)
        return false;

      if (r->load[i] > r->vehicle->capacity)
        return false;
    }

    return true;
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


  static void performCheapestFeasibleInsertion(Request *&request, Solution &solution)
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

          performEightStepEvaluationScheme(r);

          if (isFeasible(r) && r->getTotalDistance() < bestCost) {
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
    else
      r->departureTimes[i] = r->serviceBeginningTimes[i] + r->path[i]->serviceTime;
  }

  static void computeRidingTime(Route *&r, int i)
  {
    r->ridingTimes[i] = r->serviceBeginningTimes[getDeliveryIndexOf(r, i)] - r->departureTimes[i];
  }

  static void computeChargingTimes(Route *&r, int i) {
    if (r->path[i]->isStation())
      r->chargingTimes[i] = r->serviceBeginningTimes[i] - r->serviceBeginningTimes[i - 1] -
                            instance->getTravelTime(r->path[i - 1], r->path[i]);
    else
      r->chargingTimes[i] = 0.0;
  }

  static void computeBatteryLevel(Route *&r, int i)
  {
    if (r->path[i - 1]->isStation())
      r->batteryLevels[i] = r->batteryLevels[i - 1] + r->path[i]->rechargingRate * r->chargingTimes[i - 1] -
                            instance->getTravelTime(r->path[i - 1], r->path[i]);
    else
      r->batteryLevels[i] = r->batteryLevels[i - 1] - r->vehicle->dischargingRate *
                            instance->getTravelTime(r->path[i - 1], r->path[i]);
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
