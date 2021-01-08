/**
 * @file    reactive_grasp.hpp
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
  * Use reactive GRASP to solve the instance.
  *
  * @param iterations Total number of iterations.
  * @param blocks     Frequency of iterations on which probabilities are updated.
  * @param alphas     GRASP's vector of random factors.
  * @param threads    Number of threads to run.
  * @return           A Run object.
  */
  Run reactive_grasp(int iterations, int blocks, std::vector<double> alphas, int threads);

  namespace reactive_grasp_impl
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
    * Auxiliary data type to track performance of each alpha in Reactive GRASP.
    */
    struct AlphaInfo
    {
      double probability;
      double sum;
      int count;

     /**
      * Get the average value of all solutions found using the attached alpha.
      *
      * @return The average.
      */
      double avg() {
        return count > 0 ? sum/count : 0;
      }
    };

   /**
    * Choose an alpha value based in their probabilities.
    *
    * @param alphas_map A map with each alpha performance info.
    * @return          A (double) alpha value.
    */
    double get_random_alpha(std::map<double, AlphaInfo> alphas_map);

   /**
    * Construct a random greedy solution.
    *
    * @param alpha A random factor to allow restricted selection from the list of candidates.
    * @return      A Solution.
    */
    Solution construct_greedy_randomized_solution(double alpha);

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

   /**
    * Update probability of each random alpha based in the best solution found so far.
    *
    * @param alphas_map A map with each alpha performance info.
    * @param best_cost  Cost of best solution found at the moment.
    */
    void update_probs(std::map<double, AlphaInfo> *alphas_map, double best_cost);
  } // namespace reactive_grasp_impl
} // namespace algorithms

#endif // REACTIVE_GRASP_HPP_INCLUDED
