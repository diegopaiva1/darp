/**
 * @file    route.cpp
 * @author  Diego Paiva
 * @date    06/10/2019
 */

#include "route.hpp"
#include "instance.hpp"

#include "fort.hpp"

#include <cmath>   // MAXFLOAT
#include <numeric> // std::accumulate

Route::Route(Vehicle *vehicle)
{
  this->vehicle = vehicle;
}

bool Route::operator==(Route &r) const
{
  return vehicle == r.vehicle;
}

bool Route::operator!=(Route &r) const
{
  return !operator==(r);
}

void Route::evaluate()
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

  STEP1:
  departure_times[0] = path[0]->arrival_time;
  service_beginning_times[0] = departure_times[0];

  STEP2:
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
  }

  STEP3:
  forward_time_slack_at_0 = get_forward_time_slack(0);

  STEP4:
  departure_times[0] = path[0]->arrival_time + std::min(
    forward_time_slack_at_0, std::accumulate(waiting_times.begin() + 1, waiting_times.end() - 1, 0.0)
  );

  service_beginning_times[0] = departure_times[0];

  STEP5:
  for (int i = 1; i < path.size(); i++) {
    compute_arrival_time(i);
    compute_service_beginning_time(i);
    compute_waiting_time(i);
    compute_departure_time(i);
  }

  STEP6:
  for (int i = 1; i < path.size() - 1; i++)
    if (path[i]->is_pickup())
      compute_ride_time(i);

  STEP7:
  for (int j = 1; j < path.size() - 1; j++) {
    if (path[j]->is_pickup()) {
      STEP7a:
      double forward_time_slack = get_forward_time_slack(j);

      STEP7b:
      waiting_times[j] += std::min(
        forward_time_slack, std::accumulate(waiting_times.begin() + j + 1, waiting_times.end() - 1, 0.0)
      );

      service_beginning_times[j] = arrival_times[j] + waiting_times[j];
      departure_times[j] = service_beginning_times[j] + path[j]->service_time;

      STEP7c:
      for (int i = j + 1; i < path.size(); i++) {
        compute_arrival_time(i);
        compute_service_beginning_time(i);
        compute_waiting_time(i);
        compute_departure_time(i);
      }

      STEP7d:
      for (int i = j + 1; i < path.size() - 1; i++)
        if (path[i]->is_delivery())
          compute_ride_time(get_index(inst.get_request(path[i]).pickup, i));
    }
  }

  STEP8:
  cost = 0.0;
  load_violation = 0;
  time_window_violation = 0.0;
  max_ride_time_violation = 0.0;

  for (int i = 0; i < path.size(); i++) {
    if (i < path.size() - 1)
      cost += inst.get_travel_time(path[i], path[i + 1]);

    if (path[i]->is_pickup())
      max_ride_time_violation += std::max(0.0, ride_times[i] - path[i]->max_ride_time);

    load_violation += std::max(0, load[i] - vehicle->capacity);
    time_window_violation += std::max(0.0, service_beginning_times[i] - path[i]->departure_time);
    max_route_duration_violation = std::max(0.0, duration() - vehicle->max_route_duration);
  }

  cost += load_violation + time_window_violation + max_ride_time_violation + max_ride_time_violation;
}

bool Route::feasible()
{
  double violationSum = load_violation + time_window_violation + max_ride_time_violation + max_route_duration_violation;
  return std::fpclassify(violationSum) == FP_ZERO;
}

double Route::get_forward_time_slack(int i)
{
  double min_time_slack = MAXFLOAT;

  for (int j = i; j < path.size(); j++) {
    double pj = 0.0;

    if (path[j]->is_delivery() && get_index(inst.get_request(path[j]).pickup, j) < i)
      pj = ride_times[get_index(inst.get_request(path[j]).pickup, j)];

    double time_slack = std::accumulate(waiting_times.begin() + i + 1, waiting_times.begin() + j + 1, 0.0) +
                        std::max(0.0, std::min(path[j]->departure_time - service_beginning_times[j], 90.0 - pj));

    if (time_slack < min_time_slack)
      min_time_slack = time_slack;
  }

  return min_time_slack;
}

bool Route::empty()
{
  for (auto it = path.begin() + 1; it != path.end() - 1; it++)
    if ((*it)->is_pickup())
      return false;

  return true;
}

void Route::compute_load(int i)
{
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
  ride_times[i] = service_beginning_times[get_index(inst.get_request(path[i]).delivery, i)] - departure_times[i];
}

double Route::duration()
{
  return service_beginning_times.back() - service_beginning_times.front();
}

std::string Route::to_string()
{
  fort::char_table table;
  int cols = 10;

  // Decision variables
  table << std::fixed << std::setprecision(2)
        << fort::header << vehicle->id << cost << "ROUTE SCHEDULE" << fort::endr
        << "#" << "ID" << "e" << "l" << "A" << "B" << "W" << "D" << "R" << "Q" << fort::endr << fort::separator;

  table[0][2].set_cell_span(cols - 2);
  table[0][2].set_cell_text_align(fort::text_align::center);

  for (int i = 0; i < cols; i++)
    table.column(i).set_cell_text_align(fort::text_align::right);

  for (int i = 0; i < path.size(); i++) {
    int id = path[i]->id;
    double e = path[i]->arrival_time;
    double l = path[i]->departure_time;
    double A = arrival_times[i];
    double B = service_beginning_times[i];
    double W = waiting_times[i];
    double D = departure_times[i];
    double R = ride_times[i];
    int Q = load[i];

    table << i << id << e << l << A << B << W << D << R << Q << fort::endr;
  }

  return table.to_string();
}

int Route::get_index(Node *node, int start)
{
  if (node->is_delivery()) {
    // We want to find the delivery, so we must perform a forward loop
    for (int i = start; i < path.size() - 1; i++)
      if (path[i] == node)
        return i;
  }
  else {
    // Perform the loop backwards for pickups
    for (int i = start; i >= 0; i--)
      if (path[i] == node)
        return i;
  }

  throw "error";
}