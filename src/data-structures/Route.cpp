/**
 * @file    Route.hpp
 * @author  Diego Paiva
 * @date    06/10/2019
 */

#include "Route.hpp"
#include "Singleton.hpp"

Singleton *inst = Singleton::getInstance();

Route::Route(Vehicle v)
{
  vehicle = v;
}

Route::Route()
{
  // Empty constructor
}

Route::~Route()
{
  // Empty destructor
}

bool Route::isFeasible()
{
  return loadViolation == 0 && maxRideTimeViolation == 0 && timeWindowViolation == 0 &&
         finalBatteryViolation == 0 && !batteryLevelViolation;
}

void Route::printPath()
{
  printf("Route %d: ", vehicle.id);

  for (Node *node : path)
    printf("%d ", node->id);
}

void Route::printSchedule()
{
  for (int i = 0; i < path.size(); i++) {
    printf("A[%02d] = %6.4g\t", i,  arrivalTimes[i]);
    printf("B[%02d] = %6.4g\t", i,  serviceBeginningTimes[i]);
    printf("D[%02d] = %6.4g\t", i,  departureTimes[i]);
    printf("W[%02d] = %.4g\t",  i,  waitingTimes[i]);
    printf("R[%02d] = %.4g\t",  i,  ridingTimes[i]);
    printf("Z[%02d] = %.4g\t",  i,  batteryLevels[i]);
    printf("E[%02d] = %.4g\t",  i,  chargingTimes[i]);
    printf("Q[%02d] = %d",      i,  load[i]);
    printf("\n");
  }
}

void Route::performEightStepEvaluationScheme()
{
  bool  allRidingTimesRespected;
  float forwardTimeSlackAtBeginning;
  float waitingTimeSum;

  int size = path.size();
  arrivalTimes.clear();
  arrivalTimes.resize(size);
  serviceBeginningTimes.clear();
  serviceBeginningTimes.resize(size);
  departureTimes.clear();
  departureTimes.resize(size);
  waitingTimes.clear();
  waitingTimes.resize(size);
  ridingTimes.clear();
  ridingTimes.resize(size);
  load.clear();
  load.resize(size);
  batteryLevels.clear();
  batteryLevels.resize(size);
  chargingTimes.clear();
  chargingTimes.resize(size);

  STEP1:
    departureTimes[0] = serviceBeginningTimes[0];

  STEP2:
    for (int i = 0; i < path.size(); i++) {
      computeLoad(i);

      // Violated vehicle capacity, that's an irreparable violation
      if (load[i] > vehicle.capacity)
        goto STEP8;

      computeArrivalTime(i);
      computeServiceBeginningTime(i);

      // Violated time windows, that's an irreparable violation
      if (serviceBeginningTimes[i] > path[i]->departureTime)
        goto STEP8;

      computeWaitingTime(i);
      computeChargingTime(i);
      computeBatteryLevel(i);

      // Violated battery level, that's an irreparable violation
      if (batteryLevels[i] < 0.0)
        goto STEP8;

      computeDepartureTime(i);
    }

  STEP3:
    forwardTimeSlackAtBeginning = computeForwardTimeSlack(0);

  STEP4:
    waitingTimeSum = 0.0;

    for (int i = 1; i < path.size(); i++)
      waitingTimeSum += waitingTimes[i];

    departureTimes[0] = path[0]->arrivalTime + std::min(forwardTimeSlackAtBeginning, waitingTimeSum);

  STEP5:
    for (int i = 0; i < path.size(); i++) {
      computeArrivalTime(i);
      computeServiceBeginningTime(i);
      computeWaitingTime(i);
      computeChargingTime(i);
      computeDepartureTime(i);
    }

  STEP6:
    allRidingTimesRespected = true;

    for (int i = 1; i < path.size() - 1; i++)
      if (path[i]->isPickup()) {
        computeRidingTime(i);

        if (ridingTimes[i] > path[i]->maxRideTime)
          allRidingTimesRespected = false;
      }

    if (allRidingTimesRespected)
      goto STEP8;

  STEP7:
    for (int i = 1; i < path.size() - 1; i++) {
      if (path[i]->isPickup()) {
        STEP7a:
          float forwardTimeSlack = computeForwardTimeSlack(i);

        STEP7b:
          float waitingTimeSum = 0.0;

          for (int p = i + 1; p < path.size(); p++)
            waitingTimeSum += waitingTimes[p];

          waitingTimes[i] += std::min(forwardTimeSlack, waitingTimeSum);
          serviceBeginningTimes[i] = arrivalTimes[i] + waitingTimes[i];
          departureTimes[i] = serviceBeginningTimes[i] + path[i]->serviceTime;

        STEP7c:
          for (int j = i + 1; j < path.size(); j++) {
            computeArrivalTime(j);
            computeServiceBeginningTime(j);
            computeWaitingTime(j);
            computeChargingTime(j);
            computeDepartureTime(j);
          }

        STEP7d:
          allRidingTimesRespected = true;

          for (int j = i + 1; j < path.size() - 1; j++)
            if (path[j]->isDelivery()) {
              int pickupIndex = getPickupIndexOf(j);
              computeRidingTime(pickupIndex);

              if (ridingTimes[pickupIndex] > path[pickupIndex]->maxRideTime)
                allRidingTimesRespected = false;
            }

          if (allRidingTimesRespected)
            goto STEP8;
      }
    }

  STEP8:
    cost                  = 0.0;
    loadViolation         = 0;
    timeWindowViolation   = 0.0;
    maxRideTimeViolation  = 0.0;
    finalBatteryViolation = 0.0;
    batteryLevelViolation = false;

    for (int i = 0; i < path.size(); i++) {
      if (i < path.size() - 1)
        cost += 0.75 * inst->getTravelTime(path[i], path[i + 1]);

      if (path[i]->isPickup()) {
        float rideTimeExcess = ridingTimes[i] - inst->getTravelTime(path[i], path[getDeliveryIndexOf(i)]);
        cost += 0.25 * rideTimeExcess;
        maxRideTimeViolation += std::max(0.0f, ridingTimes[i] - path[i]->maxRideTime);
      }

      loadViolation += std::max(0, load[i] - vehicle.capacity);
      timeWindowViolation += std::max(0.0f, serviceBeginningTimes[i] - path[i]->departureTime);

      if (batteryLevels[i] < 0 || batteryLevels[i] > vehicle.batteryCapacity)
        batteryLevelViolation = true;
    }

    finalBatteryViolation = std::max(0.0f, vehicle.batteryCapacity * vehicle.minFinalBatteryRatioLevel - batteryLevels[path.size() - 1]);
}

float Route::computeForwardTimeSlack(int i)
{
  float forwardTimeSlack;

  for (int j = i; j < path.size(); j++) {
    float waitingTimeSum = 0.0;

    for (int p = i + 1; p <= j; p++)
      waitingTimeSum += waitingTimes[p];

    float userRideTimeWithDeliveryAtJ = 0.0;
    bool jMinusNIsVisitedBeforeIndex  = false;

    for (int p = 0; p <= i; p++)
      if (path[p]->id == j - inst->requestsAmount < i)
        jMinusNIsVisitedBeforeIndex = true;

    if (path[j]->isDelivery() && jMinusNIsVisitedBeforeIndex)
      userRideTimeWithDeliveryAtJ = ridingTimes[getPickupIndexOf(j)];

    float slackTime = waitingTimeSum + std::max(0.0f, std::min(path[j]->departureTime - serviceBeginningTimes[j],
                                                path[i]->maxRideTime - userRideTimeWithDeliveryAtJ));

    if (j == i || slackTime < forwardTimeSlack)
      forwardTimeSlack = slackTime;
  }

  return forwardTimeSlack;
}

void Route::computeLoad(int i)
{
  if (i == 0)
    load[i] = 0;
  else
    load[i] = load[i - 1] + path[i]->load;
}

void Route::computeArrivalTime(int i)
{
  if (i == 0)
    arrivalTimes[i] = 0;
  else
    arrivalTimes[i] = departureTimes[i - 1] + inst->getTravelTime(path[i - 1], path[i]);
}

void Route::computeServiceBeginningTime(int i)
{
  if (i == 0)
    serviceBeginningTimes[i] = departureTimes[i];
  else
    serviceBeginningTimes[i] = std::max(arrivalTimes[i], path[i]->arrivalTime);
}

void Route::computeWaitingTime(int i)
{
  if (i == 0)
    waitingTimes[i] = 0;
  else
    waitingTimes[i] = serviceBeginningTimes[i] - arrivalTimes[i];
}

void Route::computeDepartureTime(int i)
{
  if (path[i]->isStation())
    departureTimes[i] = serviceBeginningTimes[i] + chargingTimes[i];
  else
    departureTimes[i] = serviceBeginningTimes[i] + path[i]->serviceTime;
}

void Route::computeRidingTime(int i)
{
  ridingTimes[i] = serviceBeginningTimes[getDeliveryIndexOf(i)] - departureTimes[i];
}

void Route::computeChargingTime(int i)
{
  if (!path[i]->isStation())
    chargingTimes[i] = 0.0;
  else
    chargingTimes[i] = computeForwardTimeSlack(i + 1);
}

void Route::computeBatteryLevel(int i)
{
  if (i == 0)
    batteryLevels[i] = vehicle.initialBatteryLevel;
  else
    batteryLevels[i] = batteryLevels[i - 1] + path[i - 1]->rechargingRate * chargingTimes[i - 1] -
                          vehicle.dischargingRate * inst->getTravelTime(path[i - 1], path[i]);
}

// Retorna o índice 'i' de desembarque (delivery) de um nó 'j' de embarque (pickup) da rota
int Route::getDeliveryIndexOf(int j)
{
  if (path[j]->type != Type::PICKUP)
    throw "O nó fornecido não é um ponto de embarque";

  for (int i = 1; i < path.size(); i++)
    if (path[i]->id == path[j]->id + inst->requestsAmount)
      return i;
}

// Retorna o índice 'i' de embarque (pickup) de um nó 'j' de desembarque (delivery) da rota
int Route::getPickupIndexOf(int j)
{
  if (path[j]->type != Type::DELIVERY)
    throw "O nó fornecido não é um ponto de desembarque";

  for (int i = 1; i < path.size(); i++)
    if (path[i]->id == path[j]->id - inst->requestsAmount)
      return i;
}
