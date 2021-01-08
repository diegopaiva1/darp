/**
 * @file   instance.hpp
 * @author Diego Paiva
 * @date   25/09/2019
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

 /**
  * Pre-processing step.
  * Compute the distance between every pair of nodes and store it in the distance matrix.
  */
  void init_distance_matrix();

public:
  std::string name;
  std::vector<Node*> nodes;
  std::vector<Vehicle*> vehicles;
  std::vector<Request*> requests;
  std::vector<std::vector<double>> distance_matrix;

 /**
  * Default destructor.
  */
  ~Instance();

 /**
  * Instances should not be cloneable.
  */
  Instance(const Instance&) = delete;

 /**
  * Instances should not be assignable.
  */
  void operator=(const Instance&) = delete;

 /**
  * Get the instance.
  *
  * @return The unique instance.
  */
  static Instance& get_unique();

 /**
  * Initialize the instance with the data passed by file.
  *
  * @param instance_file_name File containing instance data.
  */
  void init(std::string instance_file_name);

 /**
  * Get depot node.
  *
  * @return Depot node.
  */
  Node* get_depot();

 /**
  * Get the request associated with a node.
  *
  * @param node The node.
  * @return     The request associated to the node.
  */
  Request* get_request(Node *node);

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
