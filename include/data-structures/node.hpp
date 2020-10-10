/**
 * @file    node.hpp
 * @author  Diego Paiva
 * @date    22/09/2019
 *
 * A class to represent a node for the DARP.
 */

#ifndef NODE_HPP_INCLUDED
#define NODE_HPP_INCLUDED

/**
 * Enumeration of possible node types.
 */
enum class Type {DEPOT, PICKUP, DELIVERY};

class Node
{
public:
  int id;
  double latitude;
  double longitude;
  double service_time;
  int load;
  double arrival_time;
  double departure_time;
  Type type;
  int index;
  double max_ride_time;

 /**
  * Default constructor.
  */
  Node() {};

 /**
  * Constructor with id.
  *
  * @param id Node's id.
  */
  Node(int id);

 /**
  * Default destructor.
  */
  ~Node() {};

 /**
  * Checks if this is pickup node.
  *
  * @return `true` if pickup node.
  */
  bool is_pickup();

 /**
  * Checks if this is delivery node.
  *
  * @return `true` if delivery node.
  */
  bool is_delivery();

 /**
  * Checks if this is depot node.
  *
  * @return `true` if depot node.
  */
  bool is_depot();
};

#endif // NODE_HPP_INCLUDED
