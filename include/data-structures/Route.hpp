/**
 * @file    Route.hpp
 * @author  Diego Paiva
 * @date    06/10/2019
 *
 * A class to represent a route for the e-ADARP.
 */

#ifndef ROUTE_HPP_INCLUDED
#define ROUTE_HPP_INCLUDED

#include "Vehicle.hpp"
#include "Request.hpp"
#include "Node.hpp"

#include <vector>

class Route
{
public:
  Vehicle *vehicle;
  std::vector<Node *> path;
  std::vector<int>    load;
  std::vector<double> arrivalTimes;
  std::vector<double> serviceBeginningTimes;
  std::vector<double> departureTimes;
  std::vector<double> waitingTimes;
  std::vector<double> rideTimes;
  std::vector<double> batteryLevels;
  std::vector<double> chargingTimes;
  std::vector<double> rideTimeExcesses;
  double travelTime;
  double excessRideTime;
  double cost;
  double loadViolation;
  double timeWindowViolation;
  double maxRideTimeViolation;
  double finalBatteryViolation;
  double batteryLevelViolation;


 /**
  * @brief Default constructor.
  */
  Route();

 /**
  * @brief Constructor with vehicle as argument.
  *
  * @param v A vehicle.
  */
  Route(Vehicle *v);

 /**
  * @brief Default destructor.
  */
  ~Route();

 /**
  * @brief Overload of '==' operator.
  *
  * @param r A route to be compared to this.
  * @return  True if r is equal this.
  */
  bool operator==(Route &r) const;

 /**
  * @brief Overload of '!=' operator.
  *
  * @param r A route to be compared to this.
  * @return  True if r is not equal this.
  */
  bool operator!=(Route &r) const;

 /**
  * @brief Check route's feasibility.
  *
  * @return True if feasible.
  */
  bool feasible();

 /**
  * @brief Perform eight-step evaluation scheme to compute costs and violations.
  *
  * @details Updates a lot of variables of the route.
  */
  void evaluate();

 /**
  * @brief The forward time slack at index i in path is the maximum amount of time that the departure
  *        from i can be delayed without violating time constraints for the later nodes.
  *
  * @param i Index of the node in the route.
  * @return  The forward time slack at index i.
  */
  double getForwardTimeSlack(int i);

 /**
  * @brief Check if route has no requests accommodated.
  *
  * @return True if empty.
  */
  bool empty();

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
  * @brief Compute the ride time of user at index i in path.
  *
  * @param i Index.
  */
  void computeRideTime(int i);

 /**
  * @brief Compute vehicle's battery level at index i in path.
  *
  * @param i Index.
  */
  void computeBatteryLevel(int i);

 /**
  * @brief Compute vehicle's charging time at index i in path.
  *
  * @param i Index.
  */
  void computeChargingTime(int i);

 /**
  * @brief Compute the ride time excess of user at index i in path.
  *
  * @param i Index.
  */
  void computeRideTimeExcess(int i);

 /**
  * @brief Get state of charge rate at index i in path.
  *
  * @param i Index.
  */
  double getStateOfCharge(int i);
};

#endif // ROUTE_HPP_INCLUDED
