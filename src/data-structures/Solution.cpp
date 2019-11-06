/**
 * @file   Solution.hpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "Solution.hpp"
#include "Singleton.hpp"

Solution::Solution()
{
  cost                  = 0.0;
  loadViolation         = 0;
  timeWindowViolation   = 0.0;
  maxRideTimeViolation  = 0.0;
  batteryLevelViolation = false;
  finalBatteryViolation = 0.0;
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
