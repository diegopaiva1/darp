/**
 * @file   request.hpp
 * @author Diego Paiva
 * @date   13/11/2018
 *
 * A class to represent a request for the DARP.
 */

#ifndef REQUEST_HPP_INCLUDED
#define REQUEST_HPP_INCLUDED

#include "node.hpp"

class Request
{
private:
 /**
  * Perform time window tightening of Request as stated in (Cordeau and Laporte, 2003).
  *
  * @param max_ride_time    Maximum ride time value.
  * @param planning_horizon Planning horizon value.
  */
  void tighten_time_windows(double max_ride_time, double planning_horizon = 1440);

public:
  Node *pickup;
  Node *delivery;

 /**
  * Constructor 1.
  */
  Request() {};

 /**
  * Constructor 2.
  */
  Request(Node *pickup, Node *delivery);

 /**
  * Default destructor.
  */
  ~Request() {};
};

#endif // REQUEST_HPP_INCLUDED
