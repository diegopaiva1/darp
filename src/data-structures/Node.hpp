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
  int index;
  double latitude;
  double longitude;
  double serviceTime;
  double maxRideTime;
  double departureTime;
  double arrivalTime;
  double rechargingRate;
  Type type;

  Node();

  Node(int id);

  ~Node();

 /**
  * @brief Get the time window's median of this node.
  *
  * @return This node's time window median.
  */
  double getTimeWindowMedian();

 /**
  * @brief Checks if this node is a pickup node.
  *
  * @return True if it is a pickup node, false otherwise.
  */
  bool isPickup();

 /**
  * @brief Checks if this node is a delivery node.
  *
  * @return True if it is a delivery node, false otherwise.
  */
  bool isDelivery();

 /**
  * @brief Checks if this node is a depot node.
  *
  * @return True if it is a depot node, false otherwise.
  */
  bool isDepot();

 /**
  * @brief Checks if this node is a station node.
  *
  * @return True if it's a station node, false otherwhise.
  */
  bool isStation();
};

#endif // NODE_HPP_INCLUDED
