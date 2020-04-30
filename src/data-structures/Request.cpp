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

Request::Request(Node *p, Node *d)
{
  pickup   = p;
  delivery = d;
}

Request::~Request()
{
  // Empty destructor
}
