/**
 * @file   vehicle.hpp
 * @author Diego Paiva
 * @date   26/09/2019
 *
 * A class to represent a vehicle for the DARP.
 */

#ifndef VEHICLE_HPP_INCLUDED
#define VEHICLE_HPP_INCLUDED

class Vehicle
{
public:
  int id;
  int capacity;
  double max_route_duration;

 /**
  * Constructor 1.
  */
  Vehicle() {};

 /**
  * Constructor 2.
  *
  * @param id                 Id
  * @param capacity           Capacity
  * @param max_route_duration Vehicle's maximum route duration
  */
  Vehicle(int id, int capacity, double max_route_duration);

 /**
  * Default destructor.
  */
  ~Vehicle() {};
};

#endif // VEHICLE_HPP_INCLUDED
