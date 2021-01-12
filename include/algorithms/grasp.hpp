/**
 * @file    grasp.hpp
 * @author  Diego Paiva
 * @date    26/09/2019
 */

#ifndef REACTIVE_GRASP_HPP_INCLUDED
#define REACTIVE_GRASP_HPP_INCLUDED

#include "run.hpp"
#include "random.hpp"

#include <map>

namespace algorithms
{
 /**
  * Use GRASP to solve the instance.
  *
  * @param iterations   Total number of iterations.
  * @param random_param GRASP's randomness parameter.
  * @param thread_count Number of threads to run.
  * @return             A Run object.
  */
  Run grasp(int iterations, double random_param, int thread_count);

  namespace details
  {
   /**
    * Get base random alias.
    */
    typedef effolkronium::random_thread_local Random;

   /**
    * Define as "Move" a method that receives a Solution and returns another Solution.
    */
    typedef Solution (*Move)(Solution s);

   /**
    * Construct a random greedy solution.
    *
    * @param random_param A random parameter in range [0, 1] to allow restricted selection from the candidate list.
    * @return             A Solution.
    */
    Solution construct_greedy_randomized_solution(double random_param);

   /**
    * Repair an infeasible solution.
    *
    * @param s A solution to be updated.
    * @return  Updated solution.
    */
    Solution repair(Solution s);

   /**
    * For a given request and a given solution, return the route (with the request inserted, if feasible)
    * of solution which results in the least increase in the objective function.
    *
    * @param req A request to be inserted.
    * @param s   A solution.
    * @return    A route with the request inserted (if resulting route is feasible).
    */
    Route get_cheapest_insertion(Request *req, Solution s);

   /**
    * For a given request and a given route, return the route configuration (with the request inserted,
    * if feasible) which results in the least increase in the objective function.
    *
    * @details If returned route has MAXFLOAT cost, then it's infeasible.
    *
    * @param req A request to be inserted.
    * @param r   A route.
    * @return    A route with the request inserted (if resulting route is feasible).
    */
    Route get_cheapest_insertion(Request *req, Route r);

   /**
    * Implementation of Variable Neighborhood Descent procedure.
    *
    * @param s              A solution to be updated.
    * @param moves          A vector containing move methods.
    * @param use_randomness Set to RVND (moves will be chosen randomly rather than the order they appear in vector).
    * @return               Updated solution.
    */
    Solution vnd(Solution s, std::vector<Move> moves, bool use_randomness = false);

   /**
    * Update a given solution by performing the "reinsert" movement.
    *
    * @param s A solution to be updated.
    * @return  Updated solution.
    */
    Solution reinsert(Solution s);

   /**
    * Update a given solution by performing the "shift(1,0)" movement.
    *
    * @param s A solution to be updated.
    * @return  Updated solution.
    */
    Solution shift_1_0(Solution s);

   /**
    * Update a given solution by performing the "2-opt*" movement.
    *
    * @param s A solution to be updated.
    * @return  Updated solution.
    */
    Solution two_opt_star(Solution s);
  } // namespace grasp_impl
} // namespace algorithms

#endif // REACTIVE_GRASP_HPP_INCLUDED
