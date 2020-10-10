/**
 * @file    instance.cpp
 * @author  Diego Paiva
 * @date    25/09/2019
 */

#include "data-structures/instance.hpp"

#include <iostream> // std::cout
#include <fstream>  // std::ifstream
#include <cmath>    // sqrt, pow

Instance& Instance::get_unique()
{
  static Instance unique;
  return unique;
}

void Instance::init(std::string instance_file_name)
{
  std::ifstream file(instance_file_name);

  if (!file.is_open()) {
    std::cout << "Failed to read file '" << instance_file_name << '\'' << std::endl;
    exit(1);
  }

  name = instance_file_name;

  // Header metadata
  int vehicles_num;
  int nodes_num;
  double max_route_duration;
  int vehicle_capacity;
  double max_ride_time;

  file >> vehicles_num;
  file >> nodes_num;
  this->requests_num = nodes_num/2;
  file >> max_route_duration;
  file >> vehicle_capacity;
  file >> max_ride_time;

  // Add vehicles
  for (int i = 1; i <= vehicles_num; i++)
    vehicles.push_back(new Vehicle(i, vehicle_capacity, max_route_duration));

  // Build all nodes
  for (int id = 0; file >> id; ) {
    Node *node = new Node(id);

    file >> node->latitude;
    file >> node->longitude;
    file >> node->service_time;
    file >> node->load;
    file >> node->arrival_time;
    file >> node->departure_time;
    node->max_ride_time = max_ride_time;

    // Add type of node
    if (node->load > 0)
      node->type = Type::PICKUP;
    else if (node->load < 0)
      node->type = Type::DELIVERY;
    else
      node->type = Type::DEPOT;

    nodes.push_back(node);
  }

  // Below code initializes the travel time matrix
  travel_times.resize(nodes.size());

  for (int i = 0; i < travel_times.size(); i++) {
    travel_times[i].resize(nodes.size());
    Node *n1 = nodes.at(i);

    for (int j = 0; j < travel_times.size(); j++) {
      Node *n2 = nodes.at(j);
      travel_times[i][j] = sqrt(pow(n1->latitude - n2->latitude, 2) + pow(n1->longitude - n2->longitude, 2));
    }
  }

  // Add all the requests
  for (int i = 1; i <= requests_num; i++)
    // Request is a pair (i, n + i)
    requests.push_back(Request(nodes.at(i), nodes.at(requests_num + i)));
}

Request Instance::get_request(Node *node)
{
  return node->is_pickup() ? requests.at(node->id - 1) : requests.at(node->id - requests_num - 1);
}

Node* Instance::get_origin_depot()
{
  return nodes.at(0);
}

Node* Instance::get_destination_depot()
{
  return nodes.at(0);
}

double Instance::get_travel_time(Node *n1, Node *n2)
{
  return travel_times[n1->id][n2->id];
}
