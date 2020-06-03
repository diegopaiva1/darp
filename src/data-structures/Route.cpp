/**
 * @file    Route.hpp
 * @author  Diego Paiva
 * @date    06/10/2019
 */

#include "data-structures/Route.hpp"
#include "data-structures/Singleton.hpp"

#include <cmath>   // MAXFLOAT
#include <numeric> // std::accumulate

#define TT_WEIGHT  0.75
#define ERT_WEIGHT 0.25

Route::~Route()
{
  // Empty destructor
}

Route::Route(Vehicle *v)
{
  vehicle = v;
}

Route::Route()
{
  // Empty constructor
}

bool Route::operator==(Route &r) const
{
  return vehicle->id == r.vehicle->id;
}

bool Route::operator!=(Route &r) const
{
  return !operator==(r);
}

void Route::evaluate()
{
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

  double forwardTimeSlackAtBeginning;

  STEP1:
  departureTimes[0]        = path[0]->arrivalTime;
  serviceBeginningTimes[0] = departureTimes[0];
  batteryLevels[0]         = vehicle->initialBatteryLevel;

  STEP2:
  for (int i = 1; i < path.size(); i++) {
    computeLoad(i);
    computeArrivalTime(i);
    computeServiceBeginningTime(i);
    computeChargingTime(i);
    computeBatteryLevel(i);

    // An irreparable violation was found
    if (load[i] > vehicle->capacity || serviceBeginningTimes[i] > path[i]->departureTime || batteryLevels[i] < 0)
      goto STEP9;

    computeWaitingTime(i);
    computeDepartureTime(i);
  }

  STEP3:
  forwardTimeSlackAtBeginning = getForwardTimeSlack(0);

  STEP4:
  departureTimes[0] = path[0]->arrivalTime + std::min(
    forwardTimeSlackAtBeginning, std::accumulate(waitingTimes.begin() + 1, waitingTimes.end(), 0.0)
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
      double forwardTimeSlack = getForwardTimeSlack(j);

      STEP7b:
      waitingTimes[j] += std::min(
        forwardTimeSlack, std::accumulate(waitingTimes.begin() + j + 1, waitingTimes.end(), 0.0)
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

  STEP8:
  for (int i = 1; i < path.size() - 1; i++)
    if (path[i]->isPickup())
      computeRideTimeExcess(i);

  STEP9:
  travelTime             = 0.0;
  excessRideTime         = 0.0;
  batteryLevelViolation  = 0.0;
  loadViolation          = 0;
  timeWindowViolation    = 0.0;
  maxRideTimeViolation   = 0.0;
  finalBatteryViolation  = 0.0;

  for (int i = 0; i < path.size(); i++) {
    if (i < path.size() - 1)
      travelTime += inst->getTravelTime(path[i], path[i + 1]);

    excessRideTime += rideTimeExcesses[i];

    if (path[i]->isPickup())
      maxRideTimeViolation += std::max(0.0, rideTimes[i] - path[i]->maxRideTime);

    loadViolation += std::max(0, load[i] - vehicle->capacity);
    timeWindowViolation += std::max(0.0, serviceBeginningTimes[i] - path[i]->departureTime);

    if (batteryLevels[i] < 0)
      batteryLevelViolation += 1000;
  }

  finalBatteryViolation += std::max(0.0, vehicle->finalMinStateOfCharge - getStateOfCharge(path.size() - 1));

  double violationSum = loadViolation + timeWindowViolation + maxRideTimeViolation +
                        finalBatteryViolation + batteryLevelViolation;

  cost = TT_WEIGHT * travelTime + ERT_WEIGHT * excessRideTime + violationSum;
}

bool Route::feasible()
{
  double violationSum = loadViolation + timeWindowViolation + maxRideTimeViolation +
                        finalBatteryViolation + batteryLevelViolation;

  return std::fpclassify(violationSum) == FP_ZERO;
}

double Route::getForwardTimeSlack(int i)
{
  double forwardTimeSlack = MAXFLOAT;

  for (int j = i; j < path.size(); j++) {
    double pj = 0.0;

    if (path[j]->isDelivery() && inst->getRequest(path[j]).pickup->index < i)
      pj = rideTimes[inst->getRequest(path[j]).pickup->index];

    double timeSlack = std::accumulate(waitingTimes.begin() + i + 1, waitingTimes.begin() + j + 1, 0.0) +
                       std::max(0.0, std::min(path[j]->departureTime - serviceBeginningTimes[j], 30.0 - pj));

    if (timeSlack < forwardTimeSlack)
      forwardTimeSlack = timeSlack;
  }

  return forwardTimeSlack;
}

bool Route::empty()
{
  for (auto it = path.begin() + 1; it != path.end() - 1; it++)
    if ((*it)->isPickup())
      return false;

  return true;
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

void Route::computeBatteryLevel(int i)
{
  batteryLevels[i] = batteryLevels[i - 1] + path[i - 1]->rechargingRate * chargingTimes[i - 1] -
                     vehicle->dischargingRate * inst->getTravelTime(path[i - 1], path[i]);
}

void Route::computeChargingTime(int i)
{
  if (path[i]->isStation()) {
    double sum = 0.0;

    for (int j = i + 1; j < path.size(); j++)
      sum += inst->getTravelTime(path[j - 1], path[j]);

    chargingTimes[i] = path[i]->departureTime - serviceBeginningTimes[i] - sum;
  }
  else {
    chargingTimes[i] = 0.0;
  }
}

void Route::computeRideTimeExcess(int i)
{
  rideTimeExcesses[i] = rideTimes[i] - inst->getTravelTime(inst->getRequest(path[i]).pickup, inst->getRequest(path[i]).delivery);
}

double Route::getStateOfCharge(int i)
{
  double battery = vehicle->initialBatteryLevel;

  for (int j = 1; j <= i; j++)
    if (path[j - 1]->isStation())
      battery += path[j - 1]->rechargingRate * chargingTimes[j - 1] -
                 vehicle->dischargingRate * inst->getTravelTime(path[j - 1], path[j]);
    else
      battery -= vehicle->dischargingRate * inst->getTravelTime(path[j - 1], path[j]);

  return battery/vehicle->batteryCapacity;
}
