/**
 * @file   Request.hpp
 * @author Diego Paiva
 * @date   13/11/2018
 */

#include "Request.hpp"

Request::Request(Node *pickup, Node *delivery)
{
  this->pickup = pickup;
  this->delivery = delivery;
}

Request::~Request()
{
  // Empty destructor
}

float Request::getTimeWindowMedian()
{
  return getCriticalNode()->getTimeWindowMedian();
}

Node* Request::getCriticalNode()
{
  return this->isInbound() ? pickup : delivery;
}

bool Request::isInbound()
{
  return !isOutbound();
}

// TODO: Melhorar a lógica que determina se a requisição é outbound
bool Request::isOutbound()
{
  return delivery->arrivalTime + delivery->departureTime != 1440;
}
