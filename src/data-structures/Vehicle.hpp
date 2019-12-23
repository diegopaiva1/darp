/**
 * @file   Vehicle.hpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#ifndef VEHICLE_HPP_INCLUDED
#define VEHICLE_HPP_INCLUDED

class Vehicle
{
public:
  int id;
  int capacity;
  double dischargingRate;
  double initialBatteryLevel;
  double batteryCapacity;
  double minFinalBatteryRatioLevel;

  Vehicle();

  Vehicle(int id);

  ~Vehicle();
};

#endif // VEHICLE_HPP_INCLUDED
