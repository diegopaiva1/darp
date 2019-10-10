/**
 * @file    Node.hpp
 * @author  Diego Paiva e Silva
 * @date    22/09/2019
 */

#ifndef NODE_H_INCLUDED
#define NODE_H_INCLUDED

#include "Point.hpp"

enum class Type {DEPOT, PICKUP, DELIVERY, STATION};

class Node
{
public:
  int id;
  int load;
  Point *point;
  int serviceTime;
  int maxRideTime;
  float departureTime;
  float arrivalTime;
  float rechargingRate;
  Type type;

  Node()
  {
    this->point = new Point();
  }

  Node(int id)
  {
    this->point = new Point(); this->id = id;
  }

  ~Node() {};

  float getTimeWindowMedian()
  {
    return (arrivalTime + departureTime)/2;
  }

  bool isPickup()
  {
    return this->type == Type::PICKUP;
  }

  bool isDelivery()
  {
    return this->type == Type::DELIVERY;
  }

  bool isDepot()
  {
    return this->type == Type::DEPOT;
  }

  bool isStation()
  {
    return this->type == Type::STATION;
  }
};

#endif // NODE_H_INCLUDED
