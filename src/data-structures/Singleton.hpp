/**
 * @file    Singleton.hpp
 * @author  Diego Paiva
 * @date    25/09/2019
 */

#ifndef SINGLETON_HPP_INCLUDED
#define SINGLETON_HPP_INCLUDED

#include <vector>
#include <fstream>
#include <limits>
#include <iostream>
#include <cmath>

#include "Node.hpp"
#include "Vehicle.hpp"
#include "Request.hpp"

class Singleton
{
private:
  inline static Singleton *instance;

  Singleton(); // Private constructor to prevent external instancing

public:
  std::string name;
  int requestsAmount;
  int stationsAmount;
  std::vector<Node *> nodes;
  std::vector<Vehicle> vehicles;
  std::vector<Request> requests;
  std::vector<std::vector<float>> travelTimes;
  std::vector<std::vector<int>> nearestStations;

 /**
  * @brief Get the instance.
  *
  * @return The unique instance, which is a singleton.
  */
  static Singleton* getInstance();

 /**
  * @brief Initialize the instance with the data passed by file.
  *
  * @param instanceFileName File where the instance is defined.
  */
  void init(std::string instanceFileName);

 /**
  * @brief Get a node by its id.
  *
  * @param id Node's id.
  * @return   The node.
  */
  Node* getNode(int id);

 /**
  * @brief Get the origin depot node.
  *
  * @return The origin depot node.
  */
  Node* getOriginDepot();

 /**
  * @brief Get the destination depot node.
  *
  * @return The destination depot node.
  */
  Node* getDestinationDepot();

 /**
  * @brief Get the request associated with a pickup node.
  *
  * @param  pickup The node.
  * @return        The request associated to the pickup node.
  */
  Request getRequest(Node *pickup);

 /**
  * @brief Get the travel time between two nodes.
  *
  * @param n1 First node.
  * @param n2 Second node.
  * @return   Travel time between n1 and n2.
  */
  float getTravelTime(Node *n1, Node *n2);
};

#endif // SINGLETON_HPP_INCLUDED
