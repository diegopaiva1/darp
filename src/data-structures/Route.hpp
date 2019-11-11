/**
 * @file    Route.hpp
 * @author  Diego Paiva
 * @date    06/10/2019
 */

#ifndef ROUTE_HPP_INCLUDED
#define ROUTE_HPP_INCLUDED

#include "Vehicle.hpp"
#include "Request.hpp"

#include <iostream>
#include <vector>

class Route
{
public:
  Vehicle vehicle;
  std::vector<Node *> path;
  std::vector<int> load;
  std::vector<float> arrivalTimes;
  std::vector<float> serviceBeginningTimes;
  std::vector<float> departureTimes;
  std::vector<float> waitingTimes;
  std::vector<float> ridingTimes;
  std::vector<float> batteryLevels;
  std::vector<float> chargingTimes;
  int loadViolation;
  float maxRideTimeViolation;
  float timeWindowViolation;
  float finalBatteryViolation;
  bool batteryLevelViolation;
  float cost;

 /**
  * @brief Default constructor.
  */
  Route();

 /**
  * @brief Constructor with vehicle as argument.
  *
  * @param v The vehicle.
  */
  Route(Vehicle v);

 /**
  * @brief Default destructor.
  */
  ~Route();

 /**
  * @brief The forward time slack at index i in path is the maximum amount of time that the departure
  *        from i can be delayed without violating time constraints for the later nodes.
  *
  * @param i Index of the node in the route.
  * @return  The forward time slack at index i.
  */
  float computeForwardTimeSlack(int i);

 /**
  * @brief The eight-step evaluation scheme is a procedure designed by (Cordeau and Laporte, 2003) for the DARP
  *        which evaluates a given route in terms of cost and feasibility. This procedure compute the routes
  *        violations, optimizes route duration and complies with ride time constraint.
  *        Note that this procedure updates all vectors of the route.
  */
  void performEightStepEvaluationScheme();

 /**
  * @brief Compute the load (number of ocuppied seats) at index i in path.
  *
  * @param i Index.
  */
  void computeLoad(int i);

 /**
  * @brief Compute vehicle's arrival time at index i in path.
  *
  * @param i Index.
  */
  void computeArrivalTime(int i);

 /**
  * @brief Compute the time which the vehicle begins its service at index i in path.
  *
  * @param i Index.
  */
  void computeServiceBeginningTime(int i);

 /**
  * @brief Compute the time which the vehicle waits before beginning its service at index i in path.
  *
  * @param i Index.
  */
  void computeWaitingTime(int i);

 /**
  * @brief Compute vehicle's departure time at index i in path.
  *
  * @param i Index.
  */
  void computeDepartureTime(int i);

 /**
  * @brief Compute the riding time of user at index in in path.
  *
  * @param i Index.
  */
  void computeRidingTime(int i);

 /**
  * @brief Compute vehicle's charging time at index i in path.
  *
  * @param i Index.
  */
  void computeChargingTime(int i);

 /**
  * @brief Compute vehicle's battery level at index i in path.
  *
  * @param i Index.
  */
  void computeBatteryLevel(int i);

  // TODO: Description
  int getDeliveryIndexOf(int i);

  // TODO: Description
  int getPickupIndexOf(int j);

 /**
  * @brief Checks if current solution is feasible (no constraints violations).
  *
  * @return True if it is feasible, false otherwise.
  */
  bool isFeasible();

 /**
  * @brief Print the nodes sequentially.
  */
  void printPath();

 /**
  * @brief Print arrival times, service beginning times, departure times, waiting times,
  *        riding times, load, battery levels and charging times for every index in this route's path.
  */
  void printSchedule();
};

#endif // ROUTE_HPP_INCLUDED
