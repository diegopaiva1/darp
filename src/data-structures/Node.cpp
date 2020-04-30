/**
 * @file    Node.cpp
 * @author  Diego Paiva
 * @date    22/09/2019
 */

#include "data-structures/Node.hpp"

Node::Node()
{
  // Empty constructor
}

Node::Node(int id)
{
  this->id = id;
}

Node::~Node()
{
  // Empty destructor
}

bool Node::isPickup()
{
  return this->type == Type::PICKUP;
}

bool Node::isDelivery()
{
  return this->type == Type::DELIVERY;
}

bool Node::isDepot()
{
  return this->type == Type::DEPOT;
}

bool Node::isStation()
{
  return this->type == Type::STATION;
}
