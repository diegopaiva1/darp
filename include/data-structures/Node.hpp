/**
 * @file    Node.hpp
 * @author  Diego Paiva
 * @date    22/09/2019
 *
 * A class to represent a node for the e-ADARP.
 */

#ifndef NODE_HPP_INCLUDED
#define NODE_HPP_INCLUDED

/**
 * @brief Enumeration of possible node types.
 */
enum class Type {DEPOT, PICKUP, DELIVERY, STATION};

class Node
{
public:
  int    id;
  int    load;
  Type   type;
  int    index;
  double latitude;
  double longitude;
  double serviceTime;
  double maxRideTime;
  double departureTime;
  double arrivalTime;
  double rechargingRate;

 /**
  * @brief Default constructor.
  */
  Node();

 /**
  * @brief Constructor with id as argument.
  */
  Node(int id);

 /**
  * @brief Default destructor.
  */
  ~Node();

 /**
  * @brief Checks if this is a pickup node.
  *
  * @return True if pickup node.
  */
  bool isPickup();

 /**
  * @brief Checks if this is a delivery node.
  *
  * @return True if delivery node.
  */
  bool isDelivery();

 /**
  * @brief Checks if this is a depot node.
  *
  * @return True if depot node.
  */
  bool isDepot();

 /**
  * @brief Checks if this is a station node.
  *
  * @return True if station node.
  */
  bool isStation();
};

#endif // NODE_HPP_INCLUDED
