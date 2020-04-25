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
 * @brief Data type to abstract a random factor in Reactive GRASP.
 */
struct RandomParam {
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
  * @brief Build a random greedy solution.
  *
  * @param alpha A random factor to allow restricted selection from the list of candidates.
  * @return      A Solution.
  */
  static Solution buildGreedyRandomizedSolution(double alpha);

  static int chooseRandomParamIndex(std::vector<RandomParam> randomParams);

  static void updateProbabilities(std::vector<RandomParam> &randomParams);

  static Route getBestInsertion(Request request, Solution solution);

 /**
  * @brief Performs the cheapest feasible insertion of a given request in a given route.
  *        If the returned route has MAX_FLOAT cost, then it's infeasible.
  *
  * @param request  Request to be inserted.
  * @param route    Route where the request will be inserted.
  * @return         The cheapest feasible route containing the request.
  */
  static Route performCheapestFeasibleInsertion(Request req, Route r);

  static Route createRoute(Solution &s);

  static Solution rvnd(Solution s);

  static Solution relocate(Solution s);

  static Solution swapZeroOne(Solution s);

  static Solution eliminate(Solution s);

  static Solution _2opt(Solution s);

  static Solution _3opt(Solution s);
};

#endif // REACTIVEGRASP_HPP_INCLUDED
