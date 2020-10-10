/**
 * @file    reactive_grasp.hpp
 * @author  Diego Paiva
 * @date    26/09/2019
 */

#ifndef REACTIVE_GRASP_HPP_INCLUDED
#define REACTIVE_GRASP_HPP_INCLUDED

#include "data-structures/run.hpp"
#include "effolkronium/random.hpp"

#include <map>

/**
 * Get base random alias.
 */
typedef effolkronium::random_static Random;

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

class ReactiveGrasp
{
public:
 /**
  * Solve the instance.
  *
  * @param iterations Total number of iterations.
  * @param blocks     Frequency of iterations on which probabilities are updated.
  * @param alphas     GRASP's vector of random factors.
  * @return           A Run object.
  */
  static Run solve(int iterations, int blocks, std::vector<double> alphas);

private:
 /**
  * Make a draw and select an alpha based in their probabilities.
  *
  * @param alphas_map A map with each alpha performance info.
  * @return          A (double) alpha value.
  */
  static double get_random_alpha(std::map<double, AlphaInfo> alphas_map);

 /**
  * Build a random greedy solution.
  *
  * @param alpha A random factor to allow restricted selection from the list of candidates.
  * @return      A Solution.
  */
  static Solution build_greedy_randomized_solution(double alpha);

 /**
  * For a given request and a given solution, return the route (with the request inserted, if feasible)
  * of solution which results in the least increase in the objective function.
  *
  * @param req A request to be inserted.
  * @param s   A solution.
  * @return    A route with the request inserted (if resulting route is feasible).
  */
  static Route get_cheapest_feasible_insertion(Request req, Solution s);

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
  static Route get_cheapest_feasible_insertion(Request req, Route r);

 /**
  * Implementation of Random Variable Neighborhood Descent procedure.
  *
  * @param s     A solution to be updated.
  * @param moves A vector containing move methods.
  * @return      Updated solution.
  */
  static Solution rvnd(Solution s, std::vector<Move> moves);

 /**
  * Update a given solution by performing the "reinsert" movement.
  *
  * @param s A solution to be updated.
  * @return  Updated solution.
  */
  static Solution reinsert(Solution s);

 /**
  * Update a given solution by performing the "swap 0-1" movement.
  *
  * @param s A solution to be updated.
  * @return  Updated solution.
  */
  static Solution swap_0_1(Solution s);

 /**
  * Update a given solution by performing the "swap 1-1" movement.
  *
  * @param s A solution to be updated.
  * @return  Updated solution.
  */
  static Solution swap_1_1(Solution s);

 /**
  * Update probability of each random alpha based in the best solution found so far.
  *
  * @param alphas_map A map with each alpha performance info.
  * @param best_cost  Cost of best solution found at the moment.
  */
  static void update_probs(std::map<double, AlphaInfo> &alphas_map, double best_cost);
};

#endif // REACTIVE_GRASP_HPP_INCLUDED
