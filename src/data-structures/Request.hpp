/**
 * @file   Request.hpp
 * @author Diego Paiva
 * @date   13/11/2018
 */

#ifndef REQUEST_HPP_INCLUDED
#define REQUEST_HPP_INCLUDED

#include "Node.hpp"

class Request
{
public:
  Node *pickup;
  Node *delivery;

  Request(Node *pickup, Node *delivery);

  ~Request();

  float getTimeWindowMedian();

  Node* getCriticalNode();

  bool isInbound();

  bool isOutbound();
};

#endif // REQUEST_HPP_INCLUDED
