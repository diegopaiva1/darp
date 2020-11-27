/**
 * @file    route.hpp
 * @author  Diego Paiva
 * @date    06/10/2019
 *
 * A class to represent a route for the DARP.
 */

#ifndef ROUTE_HPP_INCLUDED
#define ROUTE_HPP_INCLUDED

#include "vehicle.hpp"
#include "request.hpp"

#include <vector>
#include <string>

class Route
{
public:
  Vehicle *vehicle;
  std::vector<Node*> path;
  std::vector<int> load;
  std::vector<double> arrival_times;
  std::vector<double> service_beginning_times;
  std::vector<double> departure_times;
  std::vector<double> waiting_times;
  std::vector<double> ride_times;
  double cost;
  double load_violation;
  double time_window_violation;
  double max_ride_time_violation;
  double max_route_duration_violation;

 /**
  * Default constructor.
  */
  Route() {};

 /**
  * Constructor with vehicle.
  *
  * @param vehicle Vehicle.
  */
  Route(Vehicle *vehicle);

 /**
  * Default destructor.
  */
  ~Route() {};

 /**
  * Overload of '==' operator.
  *
  * @param r A route to be compared to this.
  * @return  True if r is equal this.
  */
  bool operator==(Route &r) const;

 /**
  * Overload of '!=' operator.
  *
  * @param r A route to be compared to this.
  * @return  True if r is not equal this.
  */
  bool operator!=(Route &r) const;

 /**
  * Check route's feasibility.
  *
  * @return True if feasible.
  */
  bool feasible();

 /**
  * Perform eight-step evaluation scheme to compute costs and violations.
  *
  * @details Updates a lot of variables of the route.
  */
  void evaluate();

 /**
  * Check if route has no requests accommodated.
  *
  * @return `true` if empty.
  */
  bool empty();

 /**
  * Get Route's total duration.
  *
  * @return duration value.
  */
  double duration();

 /**
  * Convert Route to string representation.
  */
  std::string to_string();

private:
 /**
  * The forward time slack at index i in path is the maximum amount of time that the departure
  * from i can be delayed without violating time constraints for the later nodes.
  *
  * @param i Index of the node in the route.
  * @return  The forward time slack at index i.
  */
  double get_forward_time_slack(int i);

 /**
  * Compute the load (number of ocuppied seats) at index i in path.
  *
  * @param i Index.
  */
  void compute_load(int i);

 /**
  * Compute vehicle's arrival time at index i in path.
  *
  * @param i Index.
  */
  void compute_arrival_time(int i);

 /**
  * Compute the time which the vehicle begins its service at index i in path.
  *
  * @param i Index.
  */
  void compute_service_beginning_time(int i);

 /**
  * Compute the time which the vehicle waits before beginning its service at index i in path.
  *
  * @param i Index.
  */
  void compute_waiting_time(int i);

 /**
  * Compute vehicle's departure time at index i in path.
  *
  * @param i Index.
  */
  void compute_departure_time(int i);

 /**
  * Compute the ride time of user at index i in path.
  *
  * @param i Index.
  */
  void compute_ride_time(int i);

  int get_index(Node *node, int start);
};

#endif // ROUTE_HPP_INCLUDED