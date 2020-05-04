/**
 * @file   Vehicle.hpp
 * @author Diego Paiva
 * @date   26/09/2019
 *
 * A class to represent a vehicle for the e-ADARP.
 */

#ifndef VEHICLE_HPP_INCLUDED
#define VEHICLE_HPP_INCLUDED

class Vehicle
{
public:
  int    id;
  int    capacity;
  double dischargingRate;
  double initialBatteryLevel;
  double batteryCapacity;
  double minFinalBatteryRatioLevel;

 /**
  * @brief Default constructor.
  */
  Vehicle();

 /**
  * @brief Constructor with id as argument.
  */
  Vehicle(int id);

 /**
  * @brief Default destructor.
  */
  ~Vehicle();
};

#endif // VEHICLE_HPP_INCLUDED
