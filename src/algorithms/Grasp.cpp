/**
 * @file   Grasp.cpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "Grasp.hpp"

Singleton *instance = Singleton::getInstance();

Solution Grasp::solve(int iterations = 1000, int iterationBlocks = 100, std::vector<float> alphas = {0.10, 0.20})
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
    std::vector<Request*> requests;

    if (it != 1) {
      alphaIndex = chooseAlphaIndex(probabilities);
      counter[alphaIndex] += 1;
    }

    if (it % iterationBlocks == 0)
      updateProbabilities(probabilities, q);

    for (Request *r : instance->requests)
      requests.push_back(r);

    for (Vehicle *v : instance->vehicles)
      currSolution.routes.push_back(new Route(v));

    for (Route *route : currSolution.routes) {
      route->path.push_back(instance->getOriginDepot());
      route->path.push_back(instance->getDestinationDepot());
    }

    while (!requests.empty()) {
      index = Prng::generateIntegerInRange(0, (int) (alphas[alphaIndex] * requests.size() - 1));
      Request *request = requests[index];
      performCheapestFeasibleInsertion(request, currSolution);
      requests.erase(requests.begin() + index);
    }

    for (Route *r : currSolution.routes) {
      currSolution.cost += performEightStepEvaluationScheme(r) +
                           r->loadViolation         * penaltyParams[0] +
                           r->timeWindowViolation   * penaltyParams[1] +
                           r->maxRideTimeViolation  * penaltyParams[2] +
                           r->batteryLevelViolation * penaltyParams[3];
                           r->finalBatteryViolation * penaltyParams[4];

      currSolution.loadViolation         += r->loadViolation;
      currSolution.maxRideTimeViolation  += r->maxRideTimeViolation;
      currSolution.timeWindowViolation   += r->timeWindowViolation;
      currSolution.batteryLevelViolation += r->batteryLevelViolation;
      currSolution.finalBatteryViolation += r->finalBatteryViolation;
    }

    // adjustPenaltyParams(currSolution, penaltyParams, delta);

    // /* Everytime a new incumbent solution is found, we randomly choose a new delta. According to
    // * (Parragh et. al, 2010) this works as a diversification mecanism and avoids cycling
    // */
    // delta = Prng::generateFloatInRange(0.05, 0.10);

    if ((it == 1) || (currSolution.routes.size() < best.routes.size()) ||
        (currSolution.routes.size() == best.routes.size() && /*currSolution.isFeasible() &&*/ currSolution.cost < best.cost))
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

    // if (it == iterations) {
    //   printf("\n\nAlphas:\n");
    //   for (int p = 0; p < probabilities.size(); p++)
    //     printf("%d. %.2f (%.2f%%) - Escolhido %d vezes\n", p + 1, alphas[p], probabilities[p], counter[p]);
    // }
  }

  std::cout << best.cost << '\n';

  return best;
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

float Grasp::performEightStepEvaluationScheme(Route *&r)
{
  bool  allRidingTimesRespected;
  float forwardTimeSlackAtBeginning;
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
      computeLoad(i, r);

      // Violated vehicle capacity, that's an irreparable violation
      if (r->load[i] > r->vehicle->capacity)
        goto STEP8;

      computeArrivalTime(i, r);
      computeServiceBeginningTime(i, r);

      // Violated time windows, that's an irreparable violation
      if (r->serviceBeginningTimes[i] > r->path[i]->departureTime)
        goto STEP8;

      computeWaitingTime(i, r);
      computeChargingTime(i, r);
      computeBatteryLevel(i, r);

      // Violated battery level, that's an irreparable violation
      if (r->batteryLevels[i] < 0.0)
        goto STEP8;

      computeDepartureTime(i, r);
    }

  STEP3:
    forwardTimeSlackAtBeginning = computeForwardTimeSlack(0, r);

  STEP4:
    waitingTimeSum = 0.0;

    for (int i = 1; i < r->path.size(); i++)
      waitingTimeSum += r->waitingTimes[i];

    r->departureTimes[0] = r->path[0]->arrivalTime + std::min(forwardTimeSlackAtBeginning, waitingTimeSum);

  STEP5:
    for (int i = 0; i < r->path.size(); i++) {
      computeArrivalTime(i, r);
      computeServiceBeginningTime(i, r);
      computeWaitingTime(i, r);
      computeChargingTime(i, r);
      computeDepartureTime(i, r);
    }

  STEP6:
    allRidingTimesRespected = true;

    for (int i = 1; i < r->path.size() - 1; i++)
      if (r->path[i]->isPickup()) {
        computeRidingTime(i, r);

        if (r->ridingTimes[i] > r->path[i]->maxRideTime)
          allRidingTimesRespected = false;
      }

    if (allRidingTimesRespected)
      goto STEP8;

  STEP7:
    for (int i = 1; i < r->path.size() - 1; i++) {
      if (r->path[i]->isPickup()) {
        STEP7a:
          float forwardTimeSlack = computeForwardTimeSlack(i, r);

        STEP7b:
          float waitingTimeSum = 0.0;

          for (int p = i + 1; p < r->path.size(); p++)
            waitingTimeSum += r->waitingTimes[p];

          r->waitingTimes[i] += std::min(forwardTimeSlack, waitingTimeSum);
          r->serviceBeginningTimes[i] = r->arrivalTimes[i] + r->waitingTimes[i];
          r->departureTimes[i] = r->serviceBeginningTimes[i] + r->path[i]->serviceTime;

        STEP7c:
          for (int j = i + 1; j < r->path.size(); j++) {
            computeArrivalTime(j, r);
            computeServiceBeginningTime(j, r);
            computeWaitingTime(j, r);
            computeChargingTime(j, r);
            computeDepartureTime(j, r);
          }

        STEP7d:
          allRidingTimesRespected = true;

          for (int j = i + 1; j < r->path.size() - 1; j++)
            if (r->path[j]->isDelivery()) {
              int pickupIndex = getPickupIndexOf(r, j);
              computeRidingTime(pickupIndex, r);

              if (r->ridingTimes[pickupIndex] > r->path[pickupIndex]->maxRideTime)
                allRidingTimesRespected = false;
            }

          if (allRidingTimesRespected)
            goto STEP8;
      }
    }

  STEP8:
    r->cost                  = 0.0;
    r->loadViolation         = 0;
    r->timeWindowViolation   = 0.0;
    r->maxRideTimeViolation  = 0.0;
    r->finalBatteryViolation = 0.0;
    r->batteryLevelViolation = false;

    for (int i = 0; i < r->path.size(); i++) {
      if (i < r->path.size() - 1)
        r->cost += 0.75 * instance->getTravelTime(r->path[i], r->path[i + 1]);

      if (r->path[i]->isPickup()) {
        float rideTimeExcess = r->ridingTimes[i] - instance->getTravelTime(r->path[i], r->path[getDeliveryIndexOf(r, i)]);

        r->cost += 0.25 * rideTimeExcess;
        r->maxRideTimeViolation += std::max(0.0f, r->ridingTimes[i] - r->path[i]->maxRideTime);
      }

      r->loadViolation += std::max(0, r->load[i] - r->vehicle->capacity);
      r->timeWindowViolation += std::max(0.0f, r->serviceBeginningTimes[i] - r->path[i]->departureTime);

      if (r->batteryLevels[i] < 0 || r->batteryLevels[i] > r->vehicle->batteryCapacity)
        r->batteryLevelViolation = true;
    }

    r->finalBatteryViolation = std::max(0.0f, r->vehicle->batteryCapacity * r->vehicle->minFinalBatteryRatioLevel - r->batteryLevels[r->path.size() - 1]);

    return r->cost;
}

float Grasp::computeForwardTimeSlack(int index, Route *r)
{
  float forwardTimeSlack;

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

    float slackTime = waitingTimeSum + std::max(0.0f, std::min(r->path[j]->departureTime - r->serviceBeginningTimes[j],
                                                r->path[index]->maxRideTime - userRideTimeWithDeliveryAtJ));

    if (j == index || slackTime < forwardTimeSlack)
      forwardTimeSlack = slackTime;
  }

  return forwardTimeSlack;
}

void Grasp::performCheapestFeasibleInsertion(Request *&request, Solution &solution)
{
  // Let us build a struct for storing intermediate stops info
  struct Stop {
    Node *station;
    int position;
  };

  // Also a struct for storing the info of the best insertion
  struct Insertion {
    Route *route;
    float cost;
    int pickupIndex;
    int deliveryIndex;
    std::vector<Stop> stops;
  };

  // Best insertion starts with infinity cost in a null route, we will update it during the search
  struct Insertion best = {nullptr, MAX_FLOAT};

  for (Route *r : solution.routes) {
    for (int p = 1; p < r->path.size(); p++) {
      r->path.insert(r->path.begin() + p, request->pickup);

      for (int d = p + 1; d < r->path.size(); d++) {
        r->path.insert(r->path.begin() + d, request->delivery);

        r->cost = performEightStepEvaluationScheme(r);

        for (int b = 0; b < r->path.size(); b++) {
          if (r->batteryLevels[b] < 0 || (b == r->path.size() - 1 && r->batteryLevels[b] < r->vehicle->batteryCapacity * r->vehicle->minFinalBatteryRatioLevel)) {
            std::vector<Stop> stops;

            for (int s = 0; s < b; s++) {
              if (r->load[s] == 0 && s != r->path.size() - 1) {
                Stop stop;
                stop.position = s + 1;
                stop.station = instance->getNode(instance->nearestStations[r->path[s]->id][r->path[s + 1]->id]);
                stops.push_back(stop);
              }
            }

            for (struct Stop &s : stops) {
              r->path.insert(r->path.begin() + s.position, s.station);
              r->cost = performEightStepEvaluationScheme(r);

              if (r->isFeasible() && r->cost < best.cost) {
                std::vector<Stop> bestStops;
                bestStops.push_back(s);
                best = {r, r->cost, p, d, bestStops};
              }

              r->path.erase(r->path.begin() + s.position);
            }
          }
        }

        if (r->isFeasible() && r->cost < best.cost)
          best = {r, r->cost, p, d};

        r->path.erase(r->path.begin() + d);
      }

      r->path.erase(r->path.begin() + p);
    }
  }

  // Request could not be feasibly inserted, so we open a new route (thus solution will be infeasible)
  if (best.route == nullptr) {
    Vehicle *v = new Vehicle(solution.routes.size() + 1);
    v->batteryCapacity = solution.routes.at(0)->vehicle->batteryCapacity;
    v->capacity = solution.routes.at(0)->vehicle->capacity;
    v->dischargingRate = solution.routes.at(0)->vehicle->dischargingRate;
    v->initialBatteryLevel = solution.routes.at(0)->vehicle->initialBatteryLevel;
    v->minFinalBatteryRatioLevel = solution.routes.at(0)->vehicle->minFinalBatteryRatioLevel;
    Route *route = new Route(v);
    route->path.push_back(instance->getOriginDepot());
    route->path.push_back(request->pickup);
    route->path.push_back(request->delivery);
    route->path.push_back(instance->getDestinationDepot());
    solution.routes.push_back(route);
  }
  else {
    best.route->path.insert(best.route->path.begin() + best.pickupIndex,   request->pickup);
    best.route->path.insert(best.route->path.begin() + best.deliveryIndex, request->delivery);

    for (struct Stop &s : best.stops)
      best.route->path.insert(best.route->path.begin() + s.position, s.station);
  }
}

void Grasp::computeLoad(int i, Route *&r)
{
  if (i == 0)
    r->load[i] = 0;
  else
    r->load[i] = r->load[i - 1] + r->path[i]->load;
}

void Grasp::computeArrivalTime(int i, Route *&r)
{
  if (i == 0)
    r->arrivalTimes[i] = 0;
  else
    r->arrivalTimes[i] = r->departureTimes[i - 1] + instance->getTravelTime(r->path[i - 1], r->path[i]);
}

void Grasp::computeServiceBeginningTime(int i, Route *&r)
{
  if (i == 0)
    r->serviceBeginningTimes[i] = r->departureTimes[i];
  else
    r->serviceBeginningTimes[i] = std::max(r->arrivalTimes[i], r->path[i]->arrivalTime);
}

void Grasp::computeWaitingTime(int i, Route *&r)
{
  if (i == 0)
    r->waitingTimes[i] = 0;
  else
    r->waitingTimes[i] = r->serviceBeginningTimes[i] - r->arrivalTimes[i];
}

void Grasp::computeDepartureTime(int i, Route *&r)
{
  if (r->path[i]->isStation())
    r->departureTimes[i] = r->serviceBeginningTimes[i] + r->chargingTimes[i];
  else
    r->departureTimes[i] = r->serviceBeginningTimes[i] + r->path[i]->serviceTime;
}

void Grasp::computeRidingTime(int i, Route *&r)
{
  r->ridingTimes[i] = r->serviceBeginningTimes[getDeliveryIndexOf(r, i)] - r->departureTimes[i];
}

void Grasp::computeChargingTime(int i, Route *&r)
{
  if (!r->path[i]->isStation())
    r->chargingTimes[i] = 0.0;
  else
    r->chargingTimes[i] = computeForwardTimeSlack(i + 1, r);
}

void Grasp::computeBatteryLevel(int i, Route *&r)
{
  if (i == 0)
    r->batteryLevels[i] = r->vehicle->initialBatteryLevel;
  else
    r->batteryLevels[i] = r->batteryLevels[i - 1] + r->path[i - 1]->rechargingRate * r->chargingTimes[i - 1] -
                          r->vehicle->dischargingRate * instance->getTravelTime(r->path[i - 1], r->path[i]);
}

// Retorna o índice 'i' de desembarque (delivery) de um nó 'j' de embarque (pickup) da rota
int Grasp::getDeliveryIndexOf(Route *&r, int j)
{
  if (r->path[j]->type != Type::PICKUP)
    throw "O nó fornecido não é um ponto de embarque";

  for (int i = 1; i < r->path.size(); i++)
    if (r->path[i]->id == r->path[j]->id + instance->requestsAmount)
      return i;
}

// Retorna o índice 'i' de embarque (pickup) de um nó 'j' de desembarque (delivery) da rota
int Grasp::getPickupIndexOf(Route *&r, int j)
{
  if (r->path[j]->type != Type::DELIVERY)
    throw "O nó fornecido não é um ponto de desembarque";

  for (int i = 1; i < r->path.size(); i++)
    if (r->path[i]->id == r->path[j]->id - instance->requestsAmount)
      return i;
}
