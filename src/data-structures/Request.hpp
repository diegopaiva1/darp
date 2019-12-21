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
  bool isInbound;
  Node *pickup;
  Node *delivery;

  Request(Node *pickup, Node *delivery);

  ~Request();

 /**
  * @brief Get the time window's median of the critical node of the request.
  *
  * @return The request's critical node's time window median.
  */
  float getTimeWindowMedian();

 /**
  * @brief Get the critical node, which is the node that has the tight time window.
  *
  * @return The request's critical node.
  */
  Node* getCriticalNode();
};

#endif // REQUEST_HPP_INCLUDED
