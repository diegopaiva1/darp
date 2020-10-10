/**
 * @file   request.hpp
 * @author Diego Paiva
 * @date   13/11/2018
 */

#include "data-structures/request.hpp"
#include "data-structures/instance.hpp"

Request::Request(Node *pickup, Node *delivery)
{
  this->pickup = pickup;
  this->delivery = delivery;
  tighten_time_windows(pickup->max_ride_time);
}

void Request::tighten_time_windows(double max_ride_time, double planning_horizon)
{
  bool inbound = pickup->departure_time - pickup->arrival_time == planning_horizon ? false : true;

  if (inbound) {
    delivery->arrival_time = std::max(
      0.0, pickup->arrival_time + pickup->service_time + inst.get_travel_time(pickup, delivery)
    );

    delivery->departure_time = std::min(
      pickup->departure_time + pickup->service_time + max_ride_time, planning_horizon
    );
  }
  else {
    pickup->arrival_time = std::max(
      0.0, delivery->arrival_time - max_ride_time - pickup->service_time
    );

    pickup->departure_time = std::min(
      delivery->departure_time - inst.get_travel_time(pickup, delivery) - pickup->service_time, planning_horizon
    );
  }
}
