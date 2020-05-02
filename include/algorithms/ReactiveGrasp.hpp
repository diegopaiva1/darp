/**
 * @file    ReactiveGrasp.hpp
 * @author  Diego Paiva
 * @date    26/09/2019
 */

#ifndef REACTIVEGRASP_HPP_INCLUDED
#define REACTIVEGRASP_HPP_INCLUDED

#include "data-structures/Run.hpp"
#include "effolkronium/random.hpp"

#include <map>

/**
 * @brief Get base random alias.
 */
typedef effolkronium::random_static Random;

/**
 * @brief Define as "Move" a method that receives a Solution and returns another Solution.
 */
typedef Solution (*Move)(Solution s);

/**
 * @brief Auxiliary data type to track performance of each alpha in Reactive GRASP.
 */
struct AlphaInfo
{
  double probability;
  double cumulativeCost;
  int    count;

 /**
  * @brief Get the average value of all solutions found using the attached alpha.
  *
  * @return The average.
  */
  double avg() {
    return count > 0 ? cumulativeCost/count : 0;
  }
};

class ReactiveGrasp
{
public:
 /**
  * @brief Solve the instance.
  *
  * @param iterations Total number of iterations.
  * @param blocks     Frequency of iterations on which probabilities are updated.
  * @param alphas     GRASP's vector of random factors.
  * @return           A Run object.
  */
  static Run solve(int iterations, int blocks, std::vector<double> alphas);

private:
 /**
  * @brief Make a draw and select an alpha based in their probabilities.
  *
  * @param alphasMap A map with each alpha performance info.
  * @return          A (double) alpha value.
  */
  static double getRandomAlpha(std::map<double, AlphaInfo> alphasMap);

 /**
  * @brief Build a random greedy solution.
  *
  * @param alpha A random factor to allow restricted selection from the list of candidates.
  * @return      A Solution.
  */
  static Solution buildGreedyRandomizedSolution(double alpha);

 /**
  * @brief For a given request and a given solution, return the route (with the request inserted, if feasible)
  *        of solution which results in the least increase in the objective function.
  *
  * @param  req  A request to be inserted.
  * @param  s    A solution.
  * @return      A route with the request inserted (if resulting route is feasible).
  */
  static Route getCheapestFeasibleInsertion(Request req, Solution s);

 /**
  * @brief For a given request and a given route, return the route configuration (with the request inserted,
  *        if feasible) which results in the least increase in the objective function.
  *
  * @details If returned route has MAXFLOAT cost, then it is infeasible.
  *
  * @param req  A request to be inserted.
  * @param r    A route.
  * @return     A route with the request inserted (if resulting route is feasible).
  */
  static Route getCheapestFeasibleInsertion(Request req, Route r);

 /**
  * @brief Implementation of Random Variable Neighborhood Descent procedure.
  *
  * @param s     A solution to be updated.
  * @param moves A vector containing move methods.
  * @return      Updated solution.
  */
  static Solution rvnd(Solution s, std::vector<Move> moves);

 /**
  * @brief Update a given solution by performing the "reinsert" movement.
  *
  * @param s A solution to be updated.
  * @return  Updated solution.
  */
  static Solution reinsert(Solution s);

 /**
  * @brief Update a given solution by performing the "swap 0-1" movement.
  *
  * @param s A solution to be updated.
  * @return  Updated solution.
  */
  static Solution swapZeroOne(Solution s);

 /**
  * @brief Update a given solution by performing the "swap 1-1" movement.
  *
  * @param s A solution to be updated.
  * @return  Updated solution.
  */
  static Solution swapOneOne(Solution s);

 /**
  * @brief Update probability of each random alpha based in the best solution found so far.
  *
  * @param alphasMap A map with each alpha performance info.
  * @param bestCost  Cost of best solution found at the moment.
  */
  static void updateProbabilities(std::map<double, AlphaInfo> &alphasMap, double bestCost);
};

#endif // REACTIVEGRASP_HPP_INCLUDED
