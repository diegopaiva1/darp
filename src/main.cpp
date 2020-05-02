/**
 * @file    main.cpp
 * @author  Diego Paiva
 * @date    24/09/2019
 */

#include "data-structures/Singleton.hpp"
#include "algorithms/ReactiveGrasp.hpp"
#include "gnuplot/Gnuplot.hpp"
#include "utils/SolutionFileStorer.hpp"

int main(const int argc, char const *argv[])
{
  const int minArgsAmount = 1;
  const int argsGiven     = argc - 1;

  if (argsGiven < minArgsAmount) {
    printf("Error: expected at least %d argument(s) - %d given\n", minArgsAmount, argsGiven);
    return EXIT_FAILURE;
  }

  inst->init(argv[1]);

  std::tuple<Solution, double, uint, int> solutionTuple = ReactiveGrasp::solve(1000, 100, {
    0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0
  });

  Solution solution = std::get<0>(solutionTuple);

  Gnuplot::plotSolution(solution);

  if (argv[2]) {
    double elapsedTime = std::get<1>(solutionTuple);
    uint seed          = std::get<2>(solutionTuple);
    uint optimalIt     = std::get<3>(solutionTuple);

    SolutionFileStorer::storeSolution(argv[2], solution, elapsedTime, seed, optimalIt);
  }

  uint seed = std::get<2>(solutionTuple);

  printf("\nSeed = %u\n", seed);

  return EXIT_SUCCESS;
}
