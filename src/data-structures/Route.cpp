/**
 * @file    Route.hpp
 * @author  Diego Paiva
 * @date    06/10/2019
 */

#include "data-structures/Route.hpp"
#include "data-structures/Singleton.hpp"

#include <cmath> // MAXFLOAT

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

bool Route::operator==(Route &r) const
{
  return vehicle.id == r.vehicle.id;
}

bool Route::operator!=(Route &r) const
{
  return !operator==(r);
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
    chargingTimes[i] = serviceBeginningTimes[i] - inst->getTravelTime(path[i - 1], path[i]) - serviceBeginningTimes[i - 1];
}

void Route::computeBatteryLevel(int i)
{
  batteryLevels[i] = batteryLevels[i - 1] + path[i - 1]->rechargingRate * chargingTimes[i - 1] -
                     vehicle.dischargingRate * inst->getTravelTime(path[i - 1], path[i]);
}
