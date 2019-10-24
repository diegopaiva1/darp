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
  std::vector<Vehicle *> vehicles;
  std::vector<Request *> requests;
  std::vector<std::vector<float>> travelTimes;
  std::vector<std::vector<float>> nearestStations;

  static Singleton* getInstance();

  // Build the nodes, vehicles and requests to this unique object through the file passed as arg
  void init(std::string instanceFileName);

  Node* getNode(int id);

  Request* getRequest(Node *pickup);

  Node* getOriginDepot();

  Node* getDestinationDepot();

  float getTravelTime(Node *n1, Node *n2);
};

#endif // SINGLETON_HPP_INCLUDED
