/**
 * @file   algorithms.hpp
 * @author Diego Paiva
 * @date   26/01/2021
 */

#ifndef ALGORITHMS_HPP_INCLUDED
#define ALGORITHMS_HPP_INCLUDED

#include "run.hpp"
#include "random.hpp"

namespace algorithms
{
 /**
  * Use GRASP to solve the instance.
  *
  * @param iterations   Total number of iterations.
  * @param random_param Randomness parameter to be used in the constructive algorithm.
  * @param thread_count Number of threads to run.
  * @return             A Run object.
  */
  Run grasp(int iterations, double random_param, int thread_count);

 /**
  * Use Iterated Local Search (ILS) to solve the instance.
  *
  * @param max_iterations            Maximum number of iterations.
  * @param no_improvement_iterations Number of iterations without improvement.
  * @param random_param              Randomness parameter to be used in the constructive algorithm.
  * @return                          A Run object.
  */
  Run ils(int max_iterations, int no_improvement_iterations, double random_param);

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
    * Generate a new starting point for the local search by perturbing solution `s`.
    *
    * @param s A solution to be perturbed.
    * @return  Updated solution.
    */
    Solution perturb(Solution s);

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
    * @param use_randomness Set to RVND (moves will be chosen randomly rather than the order they appear in vector).
    * @return               Updated solution.
    */
    Solution vnd(Solution s, bool use_randomness = false);

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
  } // namespace details
} // namespace algorithms

#endif // ALGORITHMS_HPP_INCLUDED
