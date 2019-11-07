/**
 * @file    Grasp.hpp
 * @author  Diego Paiva
 * @date    26/09/2019
 */

#ifndef GRASP_HPP_INCLUDED
#define GRASP_HPP_INCLUDED

#include <algorithm>

#include "../data-structures/Singleton.hpp"
#include "../data-structures/Solution.hpp"
#include "../utils/Prng.hpp"

#define MAX_INT   std::numeric_limits<int>::max()
#define MAX_FLOAT std::numeric_limits<float>::max()

class Grasp
{
public:
 /**
  * @brief Solve the instance.
  *
  * @param iterations      Total number of iterations.
  * @param iterationBlocks Frequency of iterations on which probabilities are updated.
  * @param alphas          GRASP's vector of random factors.
  * @return                A solution.
  */
  static Solution solve(int iterations, int iterationBlocks, std::vector<float> alphas);

private:
  static int chooseAlphaIndex(std::vector<double> probabilities);

  static void updateProbabilities(std::vector<double> &probabilities, std::vector<double> q);

 /**
  * @brief Adjust the penalty parameters according to the solution computed violations. Whenever a violation is found,
  *        it's penalty parameter is increased by the factor of (1 + delta), otherwise it is decreased by the same
  *        factor.
  *
  * @param s             A solution.
  * @param penaltyParams Vector containing the value of each penalty parameter.
  * @param delta         Random value.
  */
  static void adjustPenaltyParams(Solution s, std::vector<float> &penaltyParams, float delta);

 /**
  * @brief The eight-step evaluation scheme is a procedure designed by (Cordeau and Laporte, 2003) for the DARP
  *        which evaluates a given route in terms of cost and feasibility. This procedure compute the routes
  *        violations, optimizes route duration and complies with ride time constraint.
  *        Note that this procedure updates all vectors of route 'r'.
  *
  * @param r Route to be evaluated.
  * @return  Route's total cost.
  */
  static float performEightStepEvaluationScheme(Route &r);

 /**
  * @brief The forward time slack at index i in route r is the maximum amount of time that the departure
  *        from i can be delayed without violating time constraints for the later nodes.
  *
  * @param i Index of the node in the route.
  * @param r Route.
  * @return  The forward time slack at index i in route r.
  */
  static float computeForwardTimeSlack(int i, Route r);

 /**
  * @brief Performs the cheapest feasible insertion of a given request in a given route.
  *        If the returned route has MAX_FLOAT cost, then it's infeasible.
  *
  * @param request  Request to be inserted.
  * @param route    Route where the request will be inserted.
  * @return         The cheapest feasible route containing the request.
  */
  static Route performCheapestFeasibleInsertion(Request req, Route r);

 /**
  * @brief Compute the load (number of ocuppied seats) at index i in route r.
  *
  * @param i Index.
  * @param r Route.
  */
  static void computeLoad(int i, Route &r);

 /**
  * @brief Compute vehicle's arrival time at index i in route r.
  *
  * @param i Index.
  * @param r Route.
  */
  static void computeArrivalTime(int i, Route &r);

 /**
  * @brief Compute the time which the vehicle begins its service at index i in route r.
  *
  * @param i Index.
  * @param r Route.
  */
  static void computeServiceBeginningTime(int i, Route &r);

 /**
  * @brief Compute the time which the vehicle waits before beginning its service at index i in route r.
  *
  * @param i Index.
  * @param r Route.
  */
  static void computeWaitingTime(int i, Route &r);

 /**
  * @brief Compute vehicle's departure time at index i in route r.
  *
  * @param i Index.
  * @param r Route.
  */
  static void computeDepartureTime(int i, Route &r);

 /**
  * @brief Compute the riding time of user at index in in route r.
  *
  * @param i Index.
  * @param r Route.
  */
  static void computeRidingTime(int i, Route &r);

 /**
  * @brief Compute vehicle's charging time at index i in route r.
  *
  * @param i Index.
  * @param r Route.
  */
  static void computeChargingTime(int i, Route &r);

 /**
  * @brief Compute vehicle's battery level at index i in route r.
  *
  * @param i Index.
  * @param r Route.
  */
  static void computeBatteryLevel(int i, Route &r);

  static int getDeliveryIndexOf(Route &r, int j);

  static int getPickupIndexOf(Route &r, int j);

  static Route createRoute(Solution &s);

  static Solution localSearch(Solution &s);
};

#endif // GRASP_HPP_INCLUDED
