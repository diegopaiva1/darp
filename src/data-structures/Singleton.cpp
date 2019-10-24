/**
 * @file    Singleton.hpp
 * @author  Diego Paiva
 * @date    25/09/2019
 */

#include "Singleton.hpp"

Singleton::Singleton()
{
  // Empty constructor
}

Singleton* Singleton::getInstance()
{
  if (!instance)
    instance = new Singleton();

  return instance;
}

// Build the nodes, vehicles and requests to this unique object through the file passed as arg
void Singleton::init(std::string instanceFileName)
{
  std::ifstream file(instanceFileName);
  int vehiclesAmount;
  int originDepots;
  int destinationDepots;
  float planningHorizon;

  if (!file.is_open()) {
    printf("Failed to read file\n");
    exit(1);
  }
  else {
    name = instanceFileName;

    // Read the header of the file
    file >> vehiclesAmount;
    file >> requestsAmount;
    file >> originDepots;
    file >> destinationDepots;
    file >> stationsAmount;
    file >> planningHorizon;

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
    int nodesAmount = (2 * requestsAmount) + originDepots + destinationDepots + stationsAmount;

    // Build all instance nodes
    for (int i = 1; i <= nodesAmount; i++) {
      Node *node = new Node();
      file >> node->id;
      file >> node->latitude;
      file >> node->longitude;
      file >> node->serviceTime;
      file >> node->maxRideTime;
      file >> node->load;
      file >> node->arrivalTime;
      file >> node->departureTime;

      if (node->id == 0 || node->id == 2 * requestsAmount + 1)
        node->type = Type::DEPOT;
      else if (node->id >= 1 && node->id <= requestsAmount)
        node->type = Type::PICKUP;
      else if (node->id > requestsAmount && node->id <= 2 * requestsAmount)
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

    // Below code initializes the travel time matrix
    travelTimes.resize(nodesAmount);

    for (int i = 0; i < nodesAmount; i++) {
      travelTimes[i].resize(nodesAmount);
      Node *n1 = getNode(i);

      for (int j = 0; j < nodesAmount; j++) {
        Node *n2 = getNode(j);
        travelTimes[i][j] = sqrt(pow(n1->latitude - n2->latitude, 2) + pow(n1->longitude - n2->longitude, 2));
      }
    }

    // Below code initializes the nearest stations matrix
    nearestStations.resize(nodesAmount);

    for (int i = 0; i < nodesAmount; i++) {
      nearestStations[i].resize(nodesAmount);
      Node *n1 = getNode(i);

      for (int j = 0; j < nodesAmount; j++) {
        int nearestStationId;
        Node *n2 = getNode(j);

        float nearestStationDistance = std::numeric_limits<float>::max();

        for (int k = nodesAmount - stationsAmount; k < nodesAmount; k++) {
          if (travelTimes[i][k] + travelTimes[i][k] < nearestStationDistance) {
            nearestStationDistance = travelTimes[i][k] + travelTimes[i][k];
            nearestStationId = k;
          }
        }

        nearestStations[i][j] = nearestStationId;
      }
    }

    // Add all the requests
    for (int i = 1; i <= requestsAmount; i++) {
      Request *request = new Request(getNode(i), getNode(i + requestsAmount));
      Node *pickup   = request->pickup;
      Node *delivery = request->delivery;

      // Tigthen time windows
      if (request->isInbound()) {
        delivery->arrivalTime = std::max(
          0.0f, pickup->arrivalTime + pickup->serviceTime + getTravelTime(pickup, delivery)
        );

        delivery->departureTime = std::min(
          pickup->departureTime + pickup->maxRideTime + pickup->serviceTime, planningHorizon
        );
      }
      else {
        pickup->arrivalTime = std::max(
          0.0f, delivery->arrivalTime - pickup->maxRideTime - pickup->serviceTime
        );

        pickup->departureTime = std::min(
          delivery->departureTime - getTravelTime(pickup, delivery) - pickup->serviceTime, planningHorizon
        );
      }

      requests.push_back(request);
    }
  }
}

Node* Singleton::getNode(int id)
{
  if (nodes.at(id) != nullptr)
    return nodes.at(id);
  else
    throw "Unknown node for this instance";
}

Request* Singleton::getRequest(Node *pickup)
{
  return requests.at(pickup->id - 1);
}

Node* Singleton::getOriginDepot()
{
  return nodes.at(0);
}

Node* Singleton::getDestinationDepot()
{
  return nodes.at(2 * requestsAmount + 1);
}

float Singleton::getTravelTime(Node *n1, Node *n2)
{
  return travelTimes[n1->id][n2->id];
}