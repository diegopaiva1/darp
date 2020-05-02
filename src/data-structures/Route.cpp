/**
 * @file    Route.hpp
 * @author  Diego Paiva
 * @date    06/10/2019
 */

#include "data-structures/Route.hpp"
#include "data-structures/Singleton.hpp"

#include <cmath>   // MAXFLOAT
#include <numeric> // std::accumulate

Route::~Route()
{
  // Empty destructor
}

Route::Route(Vehicle v)
{
  vehicle = v;
}

Route::Route(Vehicle v, Request req)
{
  vehicle = v;

  path.push_back(inst->getOriginDepot());
  path.push_back(req.pickup);
  path.push_back(req.delivery);
  path.push_back(inst->getDestinationDepot());

  // Route will be definitely feasible, but we call this procedure to update its member variables
  feasible();
}

Route::Route()
{
  // Empty constructor
}

bool Route::operator==(Route &r) const
{
  return vehicle.id == r.vehicle.id;
}

bool Route::operator!=(Route &r) const
{
  return !operator==(r);
}

bool Route::feasible()
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
    batteryLevels[0]         = vehicle.initialBatteryLevel;

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
      if (batteryLevels[i] < 0.0)
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
    travelTime     = 0.0;
    excessRideTime = 0.0;

    bool   batteryLevelViolation  = false;
    int    loadViolation          = 0;
    double timeWindowViolation    = 0.0;
    double maxRideTimeViolation   = 0.0;
    double finalBatteryViolation  = 0.0;

    for (int i = 0; i < path.size(); i++) {
      if (i < path.size() - 1)
        travelTime += inst->getTravelTime(path[i], path[i + 1]);

      excessRideTime += rideTimeExcesses[i];

      if (path[i]->isPickup())
        maxRideTimeViolation += std::max(0.0, rideTimes[i] - path[i]->maxRideTime);

      loadViolation += std::max(0, load[i] - vehicle.capacity);
      timeWindowViolation += std::max(0.0, serviceBeginningTimes[i] - path[i]->departureTime);

      if (batteryLevels[i] < 0 || batteryLevels[i] > vehicle.batteryCapacity)
        batteryLevelViolation = true;
    }

    finalBatteryViolation += std::max(
      0.0, vehicle.batteryCapacity * vehicle.minFinalBatteryRatioLevel - batteryLevels[path.size() - 1]
    );

    cost = 0.75 * travelTime + 0.25 * excessRideTime;

    double violationSum = loadViolation + timeWindowViolation + maxRideTimeViolation +
                          finalBatteryViolation + batteryLevelViolation;

    return std::fpclassify(violationSum) == FP_ZERO;
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

bool Route::empty()
{
  // Empty only when path has origin and destination depots or no nodes at all
  return path.size() <= 2;
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
                     vehicle.dischargingRate * inst->getTravelTime(path[i - 1], path[i]);
}

void Route::computeChargingTime(int i)
{
  if (!path[i]->isStation())
    chargingTimes[i] = 0.0;
  else
    chargingTimes[i] = computeForwardTimeSlack(i);
}

void Route::computeRideTimeExcess(int i)
{
  rideTimeExcesses[i] = rideTimes[i] - inst->getTravelTime(inst->getRequest(path[i]).pickup, inst->getRequest(path[i]).delivery);
}
