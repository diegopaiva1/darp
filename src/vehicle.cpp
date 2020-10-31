/**
 * @file   vehicle.cpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "vehicle.hpp"

Vehicle::Vehicle(int id, int capacity, double max_route_duration)
{
  this->id = id;
  this->capacity = capacity;
  this->max_route_duration = max_route_duration;
}
