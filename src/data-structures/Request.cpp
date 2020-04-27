/**
 * @file   Request.hpp
 * @author Diego Paiva
 * @date   13/11/2018
 */

#include "data-structures/Request.hpp"

Request::Request()
{
  // Empty constructor
}

Request::Request(Node *pickup, Node *delivery)
{
  this->pickup = pickup;
  this->delivery = delivery;
}

Request::~Request()
{
  // Empty destructor
}

double Request::getTimeWindowMedian()
{
  return getCriticalNode()->getTimeWindowMedian();
}

Node* Request::getCriticalNode()
{
  return this->isInbound ? pickup : delivery;
}
