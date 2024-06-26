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
#include <unordered_map>

class Route
{
public:
  std::unordered_map<Node*, int> nodes_indices;
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
  Route();

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
  * Check if route is feasible.
  *
  * @return `true` if feasible.
  */
  bool feasible();

 /**
  * Perform eight-step evaluation scheme to compute route cost and feasibility.
  *
  * @details Updates a lot of variables of the route.
  *
  * @return `true` if feasible.
  */
  bool evaluate();

 /**
  * Check if route has no requests accommodated.
  *
  * @return `true` if empty.
  */
  bool empty();

 /**
  * Get the earliest time that node at position `i` in the route can be served.
  *
  * @return Earliest time.
  */
  double get_earliest_time(int i);

 /**
  * Get vehicle's load at position `i` in the route.
  *
  * @return Load.
  */
  int get_load(int i);

 /**
  * Get Route's total duration.
  *
  * @return duration value.
  */
  double duration();

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

 /**
  * Insert a new node in the route's path.
  *
  * @param node  The node.
  * @param index Position in the route.
  */
  void insert_node(Node *node, int index);

 /**
  * Erase node in a given position of the route's path.
  *
  * @param index Position in the route.
  */
  void erase_node(int index);

 /**
  * Erase a request in the route.
  *
  * @param request A request.
  */
  void erase_request(Request *request);
};

#endif // ROUTE_HPP_INCLUDED
