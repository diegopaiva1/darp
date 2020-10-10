/**
 * @file    instance.hpp
 * @author  Diego Paiva
 * @date    25/09/2019
 */

#ifndef INSTANCE_HPP_INCLUDED
#define INSTANCE_HPP_INCLUDED

#include <vector>
#include <string>

#include "vehicle.hpp"
#include "request.hpp"

/**
 * Macro for global access.
 */
#define inst \
        Instance::get_unique()

class Instance
{
private:
 /**
  * Unique Instance.
  */
  static Instance unique;

 /**
  * Private constructor to prevent external instancing.
  */
  Instance() {};

public:
  std::string name;
  int requests_num;
  std::vector<Node*> nodes;
  std::vector<Vehicle*> vehicles;
  std::vector<Request> requests;
  std::vector<std::vector<double>> travel_times;

 /**
  * Instances should not be cloneable.
  */
  Instance(const Instance &) = delete;

 /**
  * Instances should not be assignable.
  */
  void operator=(const Instance &) = delete;

 /**
  * Get the instance.
  *
  * @return The unique instance, which is a Instance.
  */
  static Instance& get_unique();

 /**
  * Initialize the instance with the data passed by file.
  *
  * @param instance_file_name File containing instance data.
  */
  void init(std::string instance_file_name);

 /**
  * Get the origin depot node.
  *
  * @return The origin depot node.
  */
  Node* get_origin_depot();

 /**
  * Get the destination depot node.
  *
  * @return The destination depot node.
  */
  Node* get_destination_depot();

 /**
  * Get the request associated with a node.
  *
  * @param node The node.
  * @return     The request associated to the node.
  */
  Request get_request(Node *node);

 /**
  * Get the travel time between two nodes.
  *
  * @param n1 First node.
  * @param n2 Second node.
  * @return   Travel time between n1 and n2.
  */
  double get_travel_time(Node *n1, Node *n2);
};

#endif // INSTANCE_HPP_INCLUDED
