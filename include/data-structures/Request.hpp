/**
 * @file   Request.hpp
 * @author Diego Paiva
 * @date   13/11/2018
 *
 * A class to represent a request for the e-ADARP.
 */

#ifndef REQUEST_HPP_INCLUDED
#define REQUEST_HPP_INCLUDED

#include "Node.hpp"

class Request
{
public:
  Node *pickup;
  Node *delivery;

 /**
  * @brief Default constructor.
  */
  Request();

 /**
  * @brief Constructor with pickup and delivery nodes as argument.
  */
  Request(Node *pickup, Node *delivery);

 /**
  * @brief Default destructor.
  */
  ~Request();
};

#endif // REQUEST_HPP_INCLUDED
