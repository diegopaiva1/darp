/**
 * @file   Solution.hpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "Solution.hpp"
#include "Singleton.hpp"

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

void Solution::computeCost(std::vector<float> penaltyParams)
{
  cost                  = 0.0;
  loadViolation         = 0;
  timeWindowViolation   = 0.0;
  maxRideTimeViolation  = 0.0;
  batteryLevelViolation = false;
  finalBatteryViolation = 0.0;

  for (Route &r : routes) {
    cost += r.cost + r.loadViolation         * penaltyParams[0] +
                     r.timeWindowViolation   * penaltyParams[1] +
                     r.maxRideTimeViolation  * penaltyParams[2] +
                     r.batteryLevelViolation * penaltyParams[3] +
                     r.finalBatteryViolation * penaltyParams[4];

    loadViolation         += r.loadViolation;
    timeWindowViolation   += r.timeWindowViolation;
    maxRideTimeViolation  += r.maxRideTimeViolation;
    batteryLevelViolation += r.batteryLevelViolation;
    finalBatteryViolation += r.finalBatteryViolation;
  }
}
