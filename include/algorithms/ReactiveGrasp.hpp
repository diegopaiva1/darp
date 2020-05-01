/**
 * @file    ReactiveGrasp.hpp
 * @author  Diego Paiva
 * @date    26/09/2019
 */

#ifndef REACTIVEGRASP_HPP_INCLUDED
#define REACTIVEGRASP_HPP_INCLUDED

#include "data-structures/Solution.hpp"
#include "effolkronium/random.hpp"

#include <tuple>

/**
 * @brief Get base random alias.
 */
typedef effolkronium::random_static Random;

/**
 * @brief Define as "Move" a method that receives a Solution and returns another Solution.
 */
typedef Solution (*Move)(Solution s);

/**
 * @brief Data type to abstract a random factor of Reactive GRASP.
 */
struct RandomFactor
{
  double alpha;
  double probability;
  double q;
  double cumulativeCost;
  int    count;
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
  * @return           A std::tuple containing (0) Solution, (1) elapsed time in minutes, (2) seed and (3)
  *                   number of the iterarion which optimal solution was reached.
  */
  static std::tuple<Solution, double, uint, int> solve(int iterations, int blocks, std::vector<double> alphas);

private:
 /**
  * @brief Choose a random factor in a vector of random factors and get its index in the vector.
  *
  * @param randomFactors A vector of random factors.
  * @return              Index of the chosen random factor in the vector.
  */
  static int chooseRandomFactorIndex(std::vector<RandomFactor> randomFactors);

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
  * @brief Update probability of each random factor in vector of random factors.
  *
  * @param randomFactors A vector of random factors.
  */
  static void updateProbabilities(std::vector<RandomFactor> &randomFactors);
};

#endif // REACTIVEGRASP_HPP_INCLUDED
