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
    int usersAmount;
    int originDepots;
    int destinationDepots;
    int stations;

    if (!file.is_open()) {
      printf("Failed to read file\n");
      exit(1);
    }
    else {
      file >> vehiclesAmount;
      file >> usersAmount;
      file >> originDepots;
      file >> destinationDepots;
      file >> stations;

      // Add a vehicle for each line that defines a vehicle
      for (int i = 1; i <= vehiclesAmount; i++) {
        Vehicle *vehicle = new Vehicle(i);
        file >> vehicle->capacity;
        file >> vehicle->batteryCapacity;
        file >> vehicle->initialBatteryLevel;
        file >> vehicle->minFinalBatteryRatioLevel;
        file >> vehicle->dischargingRate;
        vehicles.push_back(vehicle);
      }

      // This is always the number of nodes for dataset A
      int nodesAmount = (2 * usersAmount) + originDepots + destinationDepots + stations;

      // Build all instance nodes
      for (int i = 1; i <= nodesAmount; i++) {
        Node *node = new Node();
        file >> node->id;
        file >> node->point->x;
        file >> node->point->y;
        file >> node->serviceTime;
        file >> node->maxRideTime;
        file >> node->load;
        file >> node->arrivalTime;
        file >> node->departureTime;

        if (node->id == 0 || node->id == 2 * usersAmount + 1)
          node->type = Type::DEPOT;
        else if (node->id >= 1 && node->id <= usersAmount)
          node->type = Type::PICKUP;
        else if (node->id > usersAmount && node->id <= 2 * usersAmount)
          node->type = Type::DELIVERY;
        else
          node->type = Type::STATION;

        nodes.push_back(node);
      }

      // Finally, adding the recharging rate for every node
      for (Node *node : nodes)
        if (node->isStation())
          file >> node->rechargingRate;
        else
          node->rechargingRate = 0.0;

      travelTimes.resize(nodesAmount);
      // Fill the travel times matrix
      for (int i = 0; i < nodesAmount; i++) {
        travelTimes[i].resize(nodesAmount);

        for (int j = 0; j < nodesAmount; j++)
          travelTimes[i][j] = nodes.at(i)->point->getDistanceFrom(nodes.at(j)->point);
      }

      // Add all the requests
      for (int i = 1; i <= usersAmount; i++)
        requests.push_back(new Request(getNode(i), getNode(i + usersAmount)));
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
