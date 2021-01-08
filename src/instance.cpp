/**
 * @file   instance.cpp
 * @author Diego Paiva
 * @date   25/09/2019
 */

#include "instance.hpp"

#include <iostream> // std::cerr
#include <fstream>  // std::ifstream
#include <cmath>    // sqrt, pow

Instance::~Instance()
{
  for (Node *node : nodes)
    delete node;

  for (Vehicle *v : vehicles)
    delete v;

  for (Request *req : requests)
    delete req;
}

void Instance::init(std::string instance_file_name)
{
  std::ifstream file(instance_file_name);

  if (!file.is_open()) {
    std::cerr << "Failed to read file '" << instance_file_name << '\'' << std::endl;
    exit(1);
  }

  name = instance_file_name;

  // Header metadata
  int vehicles_num, nodes_num, vehicle_capacity;
  double max_route_duration, max_ride_time;

  file >> vehicles_num;
  file >> nodes_num;
  file >> max_route_duration;
  file >> vehicle_capacity;
  file >> max_ride_time;

  // Add vehicles
  for (int i = 1; i <= vehicles_num; i++)
    vehicles.push_back(new Vehicle(i, vehicle_capacity, max_route_duration));

  // Build all nodes
  for (int id = 0; file >> id; ) {
    Node *node = new Node(id);
    node->max_ride_time = max_ride_time;

    file >> node->latitude;
    file >> node->longitude;
    file >> node->service_time;
    file >> node->load;
    file >> node->arrival_time;
    file >> node->departure_time;

    // Add type of node
    if (node->load > 0)
      node->type = Node::Type::PICKUP;
    else if (node->load < 0)
      node->type = Node::Type::DELIVERY;
    else
      node->type = Node::Type::DEPOT;

    nodes.push_back(node);
  }

  init_distance_matrix();

  // Add all requests
  for (int i = 1, requests_num = nodes.size()/2; i <= requests_num; i++)
    // Request is a pair (i, n + i)
    requests.push_back(new Request(nodes.at(i), nodes.at(requests_num + i)));
}

void Instance::init_distance_matrix()
{
  distance_matrix.resize(nodes.size());

  for (int i = 0; i < distance_matrix.size(); i++) {
    distance_matrix[i].resize(nodes.size());

    for (int j = 0; j < distance_matrix.size(); j++) {
      distance_matrix[i][j] = sqrt(
        pow(nodes[i]->latitude - nodes[j]->latitude, 2) + pow(nodes[i]->longitude - nodes[j]->longitude, 2)
      );
    }
  }
}

Instance& Instance::get_unique()
{
  static Instance unique;
  return unique;
}

Request* Instance::get_request(Node *node)
{
  return node->is_pickup() ? requests[node->id - 1] : requests[node->id - requests.size() - 1];
}

Node* Instance::get_depot()
{
  return nodes[0];
}

double Instance::get_travel_time(Node *n1, Node *n2)
{
  return distance_matrix[n1->id][n2->id];
}
