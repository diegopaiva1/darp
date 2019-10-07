/**
 * @file    Instance.hpp
 * @author  Diego Paiva e Silva
 * @date    25/09/2019
 */

#ifndef INSTANCE_H_INCLUDED
#define INSTANCE_H_INCLUDED

#include <vector>
#include <fstream>
#include <iostream>

#include "Node.hpp"
#include "Vehicle.hpp"
#include "Request.hpp"

class Instance
{
public:
  std::vector<Node *> nodes;
  std::vector<Vehicle *> vehicles;
  std::vector<Request *> requests;
  std::vector<std::vector<float>> travelTimes;

  Instance(std::string instanceFileName) { mount(instanceFileName); };

  ~Instance() {};

  // Build the nodes, vehicles and requests to this object through the instance file passed as arg
  void mount(std::string instanceFileName)
  {
    std::ifstream file(instanceFileName);
    int vehiclesAmount;
    int requestsAmount;
    int originDepots;
    int destinationDepots;
    int stations;
    int replications;
    int timeHorizon;

    if (!file.is_open()) {
      printf("Failed to read file\n");
      exit(1);
    }
    else {
      file >> vehiclesAmount;
      file >> requestsAmount;
      file >> originDepots;
      file >> destinationDepots;
      file >> stations;
      file >> replications;
      file >> timeHorizon;

      for (int i = 1; i <= vehiclesAmount; i++) {
        Vehicle *vehicle = new Vehicle(i);
        vehicles.push_back(vehicle);
      }

      int nodesAmount = 2 * (requestsAmount + vehiclesAmount) + originDepots + destinationDepots + stations;

      for (int i = 0; i < nodesAmount; i++) {
        Node *node = new Node();
        file >> node->id;
        file >> node->point->x;
        file >> node->point->y;
        file >> node->serviceTime;
        file >> node->load;
        file >> node->arrivalTime;
        file >> node->departureTime;

        if (node->id >= 1 && node->id <= requestsAmount)
          node->type = Type::PICKUP;
        else if (node->id > requestsAmount && node->id <= 2 * requestsAmount)
          node->type = Type::DELIVERY;

        nodes.push_back(node);
      }

      int id;
      file >> id;
      nodes.at(id - 1)->type = Type::DEPOT;
      file >> id;
      nodes.at(id - 1)->type = Type::DEPOT;

      for (int i = 0; i < 2; i++) {
        for (int j = 0; j < vehiclesAmount; j++) {
          file >> id;
          nodes.at(id - 1)->type = Type::DEPOT;
        }
      }

      for (int i = 0; i < stations; i++) {
        file >> id;
        nodes.at(id - 1)->type = Type::STATION;
      }

      for (int i = 0; i < requestsAmount; i++)
        file >> nodes.at(i)->maxRideTime;

      for (int i = 0; i < 4; i++) {
        for (Vehicle *v : vehicles) {
          if (i == 0)
            file >> v->capacity;
          else if (i == 1)
            file >> v->initialBatteryLevel;
          else if (i == 2)
            file >> v->batteryCapacity;
          else
            file >> v->minFinalBatteryRatioLevel;
        }
      }

    for (Node *n : nodes)
      if (n->isStation())
        file >> n->rechargingRate;
      else
        n->rechargingRate = 0.0;

    float dischargingRate;
    file >> dischargingRate;

    for (Vehicle *v : vehicles)
      v->dischargingRate = dischargingRate;
    }
  }

  Node* getNode(int id)
  {
    if (hasNode(id)) return nodes.at(id); else throw "Unknown node for this instance";
  }

  Request* getRequest(Node *pickup)
  {
    return requests.at(pickup->id - 1);
  }

  Node* getDepartureDepot()
  {
    return nodes.front();
  }

  Node* getArrivalDepot()
  {
    return nodes.back();
  }

  float getTravelTime(Node *n1, Node *n2)
  {
    return travelTimes[n1->id][n2->id];
  }
private:
  bool hasNode(int id)
  {
    return nodes.at(id) != nullptr;
  }
};

#endif // INSTANCE_H_INCLUDED
