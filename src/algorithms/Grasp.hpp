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
  * @brief Performs the cheapest feasible insertion of a given request in a given route.
  *        If the returned route has MAX_FLOAT cost, then it's infeasible.
  *
  * @param request  Request to be inserted.
  * @param route    Route where the request will be inserted.
  * @return         The cheapest feasible route containing the request.
  */
  static Route performCheapestFeasibleInsertion(Request req, Route r);

  static Route createRoute(Solution &s);

  static Solution localSearch(Solution s, std::vector<float> penaltyParams);

  static Solution relocate(Solution s, std::vector<float> penaltyParams);

  static Solution swapZeroOne(Solution s, std::vector<float> penaltyParams);

  static Solution eliminate(Solution s, std::vector<float> penaltyParams);

  static Solution _2opt(Solution s, std::vector<float> penaltyParams);

  static Solution _3opt(Solution s, std::vector<float> penaltyParams);
};

#endif // GRASP_HPP_INCLUDED
