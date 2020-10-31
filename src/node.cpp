/**
 * @file    node.cpp
 * @author  Diego Paiva
 * @date    22/09/2019
 */

#include "node.hpp"

Node::Node(int id)
{
  this->id = id;
}

bool Node::is_pickup()
{
  return type == Type::PICKUP;
}

bool Node::is_delivery()
{
  return type == Type::DELIVERY;
}

bool Node::is_depot()
{
  return type == Type::DEPOT;
}
