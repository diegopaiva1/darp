/**
 * @file    Grasp.hpp
 * @author  Diego Paiva
 * @date    26/09/2019
 */

#ifndef GRASP_HPP_INCLUDED
#define GRASP_HPP_INCLUDED

#include "../data-structures/Singleton.hpp"
#include "../data-structures/Solution.hpp"
#include "../utils/Prng.hpp"

class Grasp
{
public:
 /**
  * @brief Solve the instance.
  *
  * @param iterations Total number of iterations.
  * @param blocks     Frequency of iterations on which probabilities are updated.
  * @param alphas     GRASP's vector of random factors.
  * @return           A solution.
  */
  static Solution solve(int iterations, int blocks, std::vector<double> alphas);

private:
  static int chooseAlphaIndex(std::vector<double> probabilities);

  static void updateProbabilities(std::vector<double> &probabilities, std::vector<double> q);

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

  static Solution localSearch(Solution s);

  static Solution relocate(Solution s);

  static Solution swapZeroOne(Solution s);

  static Solution eliminate(Solution s);

  static Solution _2opt(Solution s);

  static Solution _3opt(Solution s);
};

#endif // GRASP_HPP_INCLUDED
