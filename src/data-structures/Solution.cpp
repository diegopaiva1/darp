/**
 * @file   Solution.hpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "data-structures/Solution.hpp"
#include "data-structures/Singleton.hpp"

// #include <iomanip>
#include <fstream>

Solution::Solution()
{
  // Empty constructor
}

Solution::~Solution()
{
  // Empty destructor
}

bool Solution::isFeasible()
{
  return loadViolation         == 0 &&
         timeWindowViolation   == 0 &&
         maxRideTimeViolation  == 0 &&
         finalBatteryViolation == 0 &&
         !batteryLevelViolation     &&
         routes.size() <= Singleton::getInstance()->vehicles.size();
}

void Solution::computeCost()
{
  travelTime            = 0.0;
  excessRideTime        = 0.0;
  cost                  = 0.0;
  loadViolation         = 0;
  timeWindowViolation   = 0.0;
  maxRideTimeViolation  = 0.0;
  batteryLevelViolation = false;
  finalBatteryViolation = 0.0;

  for (Route &r : routes) {
    travelTime     += r.travelTime;
    excessRideTime += r.excessRideTime;

    loadViolation         += r.loadViolation;
    timeWindowViolation   += r.timeWindowViolation;
    maxRideTimeViolation  += r.maxRideTimeViolation;
    batteryLevelViolation += r.batteryLevelViolation;
    finalBatteryViolation += r.finalBatteryViolation;

    cost += r.cost + loadViolation + timeWindowViolation + maxRideTimeViolation +
                     batteryLevelViolation + finalBatteryViolation;
  }
}
