/**
 * @file    Route.hpp
 * @author  Diego Paiva
 * @date    06/10/2019
 */

#include <numeric>

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
  return loadViolation          == 0 &&
         maxRideTimeViolation   == 0 &&
         timeWindowViolation    == 0 &&
         finalBatteryViolation  == 0 &&
         orderViolation         == 0 &&
        //  chargingPlaceViolation == 0 &&
         !batteryLevelViolation;
}

void Route::printSchedule()
{
  for (int i = 0; i < path.size(); i++) {
    printf("[%d] %2d\t", i, path[i]->id);
    printf("A = %6.4g\t", arrivalTimes[i]);
    printf("B = %6.4g\t", serviceBeginningTimes[i]);
    printf("W = %6.4g\t", waitingTimes[i]);
    printf("D = %6.4g\t", departureTimes[i]);
    printf("L = %6.4g\t", rideTimes[i]);
    printf("Z = %6.4g\t", batteryLevels[i]);
    printf("E = %6.4g\t", chargingTimes[i]);
    printf("Q = %6d\t",   load[i]);
    printf("R = %6.4g",   rideTimeExcesses[i]);
    printf("\n");
  }
}

void Route::performEightStepEvaluationScheme()
{
  double forwardTimeSlackAtBeginning;
  int size = path.size();

  arrivalTimes.clear();
  arrivalTimes.resize(size);
  serviceBeginningTimes.clear();
  serviceBeginningTimes.resize(size);
  departureTimes.clear();
  departureTimes.resize(size);
  waitingTimes.clear();
  waitingTimes.resize(size);
  rideTimes.clear();
  rideTimes.resize(size);
  load.clear();
  load.resize(size);
  batteryLevels.clear();
  batteryLevels.resize(size);
  chargingTimes.clear();
  chargingTimes.resize(size);
  rideTimeExcesses.clear();
  rideTimeExcesses.resize(size);

  // Compute every node index in the route before evaluation
  for (int i = 1; i < path.size() - 1; i++)
    path[i]->index = i;

  STEP1:
    departureTimes[0] = path[0]->arrivalTime;
    serviceBeginningTimes[0] = departureTimes[0];
    batteryLevels[0] = vehicle.initialBatteryLevel;

  STEP2:
    for (int i = 1; i < path.size(); i++) {
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
      if (batteryLevels[i] < 0.0 /* || batteryLevels[i] > vehicle.batteryCapacity */)
        goto STEP8;

      computeDepartureTime(i);
    }

  STEP3:
    forwardTimeSlackAtBeginning = computeForwardTimeSlack(0);

  STEP4:
    departureTimes[0] = path[0]->arrivalTime + std::min(
      forwardTimeSlackAtBeginning, std::accumulate(waitingTimes.begin() + 1, waitingTimes.end() - 1, 0.0)
    );
    serviceBeginningTimes[0] = departureTimes[0];

  STEP5:
    for (int i = 1; i < path.size(); i++) {
      computeArrivalTime(i);
      computeServiceBeginningTime(i);
      computeWaitingTime(i);
      computeChargingTime(i);
      computeDepartureTime(i);
    }

  STEP6:
    for (int i = 1; i < path.size() - 1; i++)
      if (path[i]->isPickup())
        computeRideTime(i);

  STEP7:
    for (int j = 1; j < path.size() - 1; j++) {
      if (path[j]->isPickup() && load[j] == 1) {
        STEP7a:
          double forwardTimeSlack = computeForwardTimeSlack(j);

        STEP7b:
          waitingTimes[j] += std::min(
            forwardTimeSlack, std::accumulate(waitingTimes.begin() + j + 1, waitingTimes.end() - 1, 0.0)
          );
          serviceBeginningTimes[j] = arrivalTimes[j] + waitingTimes[j];
          departureTimes[j] = serviceBeginningTimes[j] + path[j]->serviceTime;

        STEP7c:
          for (int i = j + 1; i < path.size(); i++) {
            computeArrivalTime(i);
            computeServiceBeginningTime(i);
            computeWaitingTime(i);
            computeChargingTime(i);
            computeDepartureTime(i);
          }

        STEP7d:
          for (int i = j + 1; i < path.size() - 1; i++)
            if (path[i]->isDelivery())
              computeRideTime(inst->getRequest(path[i]).pickup->index);
      }
    }

  for (int i = 0; i < path.size(); i++)
    if (path[i]->isPickup())
      computeRideTimeExcess(i);

  STEP8:
    cost                   = 0.0;
    travelTime             = 0.0;
    excessRideTime         = 0.0;
    loadViolation          = 0;
    timeWindowViolation    = 0.0;
    maxRideTimeViolation   = 0.0;
    finalBatteryViolation  = 0.0;
    orderViolation         = 0;
    // chargingPlaceViolation = 0;
    batteryLevelViolation  = false;

    for (int i = 0; i < path.size(); i++) {
      if (i < path.size() - 1)
        travelTime += inst->getTravelTime(path[i], path[i + 1]);

      excessRideTime += rideTimeExcesses[i];

      if (path[i]->isPickup()) {
        maxRideTimeViolation += std::max(0.0, rideTimes[i] - path[i]->maxRideTime);

        if (inst->getRequest(path[i]).delivery->index < i)
          orderViolation += 1000;
      }

      // if (path[i]->isStation() && load[i] > 0)
      //   chargingPlaceViolation += 1000;

      loadViolation += std::max(0, load[i] - vehicle.capacity);
      timeWindowViolation += std::max(0.0, serviceBeginningTimes[i] - path[i]->departureTime);

      if (batteryLevels[i] < 0 || batteryLevels[i] > vehicle.batteryCapacity)
        batteryLevelViolation = true;
    }

    finalBatteryViolation += std::max(0.0, vehicle.batteryCapacity * vehicle.minFinalBatteryRatioLevel - batteryLevels[path.size() - 1]);
    cost = 0.75 * travelTime + 0.25 * excessRideTime;
}

double Route::computeForwardTimeSlack(int i)
{
  double forwardTimeSlack = MAXFLOAT;

  for (int j = i; j < path.size(); j++) {
    double waitingTimeSum = 0.0;
    double pj = 0.0;

    for (int p = i + 1; p <= j; p++)
      waitingTimeSum += waitingTimes[p];

    if (path[j]->isDelivery() && inst->getRequest(path[j]).pickup->index < i)
      pj = rideTimes[inst->getRequest(path[j]).pickup->index];

    double timeSlack = waitingTimeSum + std::max(0.0, std::min(path[j]->departureTime - serviceBeginningTimes[j], 30.0 - pj));

    if (timeSlack < forwardTimeSlack)
      forwardTimeSlack = timeSlack;
  }

  return forwardTimeSlack;
}

void Route::computeLoad(int i)
{
  load[i] = load[i - 1] + path[i]->load;
}

void Route::computeArrivalTime(int i)
{
  arrivalTimes[i] = departureTimes[i - 1] + inst->getTravelTime(path[i - 1], path[i]);
}

void Route::computeServiceBeginningTime(int i)
{
  serviceBeginningTimes[i] = std::max(arrivalTimes[i], path[i]->arrivalTime);
}

void Route::computeWaitingTime(int i)
{
  waitingTimes[i] = serviceBeginningTimes[i] - arrivalTimes[i];
}

void Route::computeDepartureTime(int i)
{
  if (path[i]->isStation())
    departureTimes[i] = serviceBeginningTimes[i] + chargingTimes[i];
  else
    departureTimes[i] = serviceBeginningTimes[i] + path[i]->serviceTime;
}

void Route::computeRideTime(int i)
{
  rideTimes[i] = serviceBeginningTimes[inst->getRequest(path[i]).delivery->index] - departureTimes[i];
}

void Route::computeRideTimeExcess(int i)
{
  rideTimeExcesses[i] = rideTimes[i] - inst->getTravelTime(inst->getRequest(path[i]).pickup, inst->getRequest(path[i]).delivery);
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
  batteryLevels[i] = batteryLevels[i - 1] + path[i - 1]->rechargingRate * chargingTimes[i - 1] -
                     vehicle.dischargingRate * inst->getTravelTime(path[i - 1], path[i]);
}
