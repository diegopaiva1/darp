/**
 * @file    Node.hpp
 * @author  Diego Paiva
 * @date    22/09/2019
 */

#ifndef NODE_HPP_INCLUDED
#define NODE_HPP_INCLUDED

enum class Type {DEPOT, PICKUP, DELIVERY, STATION};

class Node
{
public:
  int id;
  int load;
  float latitude;
  float longitude;
  float serviceTime;
  float maxRideTime;
  float departureTime;
  float arrivalTime;
  float rechargingRate;
  Type type;

  Node();

  Node(int id);

  ~Node();

  float getTimeWindowMedian();

  bool isPickup();

  bool isDelivery();

  bool isDepot();

  bool isStation();
};

#endif // NODE_HPP_INCLUDED
