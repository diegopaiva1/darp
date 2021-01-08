/**
 * @file   node.hpp
 * @author Diego Paiva
 * @date   22/09/2019
 *
 * A class to represent a node for the DARP.
 */

#ifndef NODE_HPP_INCLUDED
#define NODE_HPP_INCLUDED

class Node
{
public:
 /**
  * Enumeration of possible node types.
  */
  enum class Type {DEPOT, PICKUP, DELIVERY};

  int id;
  double latitude;
  double longitude;
  double service_time;
  int load;
  double arrival_time;
  double departure_time;
  Type type;
  double max_ride_time;

 /**
  * Constructor 1.
  */
  Node() {};

 /**
  * Constructor 2.
  *
  * @param id Id.
  */
  Node(int id);

 /**
  * Default destructor.
  */
  ~Node() {};

 /**
  * Checks if this node is a pickup.
  *
  * @return `true` if pickup node.
  */
  bool is_pickup();

 /**
  * Checks if this node is a delivery.
  *
  * @return `true` if delivery node.
  */
  bool is_delivery();

 /**
  * Checks if this node is a depot.
  *
  * @return `true` if depot node.
  */
  bool is_depot();
};

#endif // NODE_HPP_INCLUDED
