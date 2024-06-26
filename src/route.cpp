/**
 * @file    route.cpp
 * @author  Diego Paiva
 * @date    06/10/2019
 */

#include "route.hpp"
#include "instance.hpp"

#include <cfloat>  // FLT_MAX
#include <numeric> // std::accumulate
#include <algorithm>

Route::Route()
{
  this->cost = 0.0;
}

Route::Route(Vehicle *vehicle)
{
  this->cost = 0.0;
  this->vehicle = vehicle;
}

bool Route::feasible()
{
  return cost < FLT_MAX;
}

int Route::get_load(int i)
{
  if (i == 0)
    return 0;
  else
    return get_load(i - 1) + path[i]->load;
}

double Route::get_earliest_time(int i)
{
  if (i == 0)
    return path[i]->arrival_time;
  else
    return std::max(
      path[i]->arrival_time,
      get_earliest_time(i - 1) + path[i - 1]->service_time + inst.get_travel_time(path[i - 1], path[i])
    );
}

bool Route::evaluate()
{
  int size = path.size();

  arrival_times.clear();
  arrival_times.resize(size);
  service_beginning_times.clear();
  service_beginning_times.resize(size);
  departure_times.clear();
  departure_times.resize(size);
  waiting_times.clear();
  waiting_times.resize(size);
  ride_times.clear();
  ride_times.resize(size);
  load.clear();
  load.resize(size);

  double forward_time_slack_at_0;

  nodes_indices[path[0]] = 0;

  // STEP 1
  departure_times[0] = path[0]->arrival_time;
  service_beginning_times[0] = departure_times[0];

  // STEP 2
  for (int i = 1; i < path.size(); i++) {
    compute_load(i);

    // Violated vehicle capacity, which is an irreparable violation
    if (load[i] > vehicle->capacity)
      goto STEP8;

    compute_arrival_time(i);
    compute_service_beginning_time(i);

    // Violated time window, which is an irreparable violation
    if (service_beginning_times[i] > path[i]->departure_time)
      goto STEP8;

    compute_waiting_time(i);
    compute_departure_time(i);

    nodes_indices[path[i]] = i;
  }

  // STEP 3
  forward_time_slack_at_0 = get_forward_time_slack(0);

  // STEP 4
  departure_times[0] = path[0]->arrival_time + std::min(
    forward_time_slack_at_0, std::accumulate(waiting_times.begin() + 1, waiting_times.end() - 1, 0.0)
  );

  service_beginning_times[0] = departure_times[0];

  // STEP 5
  for (int i = 1; i < path.size(); i++) {
    compute_arrival_time(i);
    compute_service_beginning_time(i);
    compute_waiting_time(i);
    compute_departure_time(i);
  }

  // STEP 6
  for (int i = 1; i < path.size() - 1; i++)
    if (path[i]->is_pickup())
      compute_ride_time(i);

  // STEP 7
  for (int j = 1; j < path.size() - 1; j++) {
    if (path[j]->is_pickup()) {
      // STEP 7 (a)
      double forward_time_slack = get_forward_time_slack(j);

      // STEP 7 (b)
      waiting_times[j] += std::min(
        forward_time_slack, std::accumulate(waiting_times.begin() + j + 1, waiting_times.end() - 1, 0.0)
      );

      service_beginning_times[j] = arrival_times[j] + waiting_times[j];
      departure_times[j] = service_beginning_times[j] + path[j]->service_time;

      // STEP 7 (c):
      for (int i = j + 1; i < path.size(); i++) {
        compute_arrival_time(i);
        compute_service_beginning_time(i);
        compute_waiting_time(i);
        compute_departure_time(i);
      }

      // STEP 7 (d):
      for (int i = j + 1; i < path.size() - 1; i++)
        if (path[i]->is_delivery())
          compute_ride_time(nodes_indices[inst.get_request(path[i])->pickup]);
    }
  }

  STEP8:
  cost = 0.0;
  load_violation = 0;
  time_window_violation = 0.0;
  max_ride_time_violation = 0.0;

  for (int i = 1; i < path.size(); i++) {
    cost += inst.get_travel_time(path[i - 1], path[i]);
    load_violation += std::max(0, load[i] - vehicle->capacity);
    time_window_violation += std::max(0.0, service_beginning_times[i] - path[i]->departure_time);
    max_route_duration_violation = std::max(0.0, duration() - vehicle->max_route_duration);

    if (path[i]->is_pickup())
      max_ride_time_violation += std::max(0.0, ride_times[i] - path[i]->max_ride_time);
  }

  // Feasible if and only there are no violations
  return load_violation == 0 && time_window_violation == 0.0 &&
         max_ride_time_violation == 0.0 && max_route_duration_violation == 0.0;
}

double Route::get_forward_time_slack(int i)
{
  double min_time_slack = FLT_MAX;

  for (int j = i; j < path.size(); j++) {
    double pj = 0.0;

    if (path[j]->is_delivery() && nodes_indices[inst.get_request(path[j])->pickup] < i)
      pj = ride_times[nodes_indices[inst.get_request(path[j])->pickup]];

    double time_slack = std::accumulate(waiting_times.begin() + i + 1, waiting_times.begin() + j + 1, 0.0) +
                        std::max(0.0, std::min(path[j]->departure_time - service_beginning_times[j], 90.0 - pj));

    if (time_slack < min_time_slack)
      min_time_slack = time_slack;
  }

  return min_time_slack;
}

bool Route::empty()
{
  // Empty only if both origin and destination depots are present or if there are no nodes at all
  return path.size() <= 2;
}

void Route::compute_load(int i)
{
  if (i == 0)
    load[i] = 0;
  else
    load[i] = load[i - 1] + path[i]->load;
}

void Route::compute_arrival_time(int i)
{
  arrival_times[i] = departure_times[i - 1] + inst.get_travel_time(path[i - 1], path[i]);
}

void Route::compute_service_beginning_time(int i)
{
  service_beginning_times[i] = std::max(arrival_times[i], path[i]->arrival_time);
}

void Route::compute_waiting_time(int i)
{
  waiting_times[i] = service_beginning_times[i] - arrival_times[i];
}

void Route::compute_departure_time(int i)
{
  departure_times[i] = service_beginning_times[i] + path[i]->service_time;
}

void Route::compute_ride_time(int i)
{
  ride_times[i] = service_beginning_times[nodes_indices[inst.get_request(path[i])->delivery]] - departure_times[i];
}

double Route::duration()
{
  return service_beginning_times.back() - service_beginning_times.front();
}

void Route::insert_node(Node *node, int index)
{
  if (index > 0) {
    path.insert(path.begin() + index, node);

    // Recalculate route's total cost
    cost = cost + inst.get_travel_time(path[index - 1], path[index])
                + inst.get_travel_time(path[index], path[index + 1])
                - inst.get_travel_time(path[index - 1], path[index + 1]);
  }
}

void Route::erase_node(int index)
{
  if (index > 0 && index < path.size() - 1) {
    // Recalculate route's total cost
    cost = cost - inst.get_travel_time(path[index - 1], path[index])
                - inst.get_travel_time(path[index], path[index + 1])
                + inst.get_travel_time(path[index - 1], path[index + 1]);

    path.erase(path.begin() + index);
  }
}

void Route::erase_request(Request *request)
{
  int pickup_index, delivery_index;

  for (int i = path.size() - 2; i > 0; i--) {
    if (path[i] == request->pickup) {
      pickup_index = i;

      // Once the pickup index has been spotted we can break, since it always comes before the delivery node
      break;
    }
    else if (path[i] == request->delivery) {
      delivery_index = i;
    }
  }

  erase_node(delivery_index);
  erase_node(pickup_index);
}
