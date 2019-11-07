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

 /**
  * @brief Check if this request is a inbound request.
  *
  * @return True if it is a inbound request, false otherwise.
  */
  bool isInbound();

 /**
  * @brief Check if this request is a outbound request.
  *
  * @return True if it is a outbound request, false otherwise.
  */
  bool isOutbound();
};

#endif // REQUEST_HPP_INCLUDED
