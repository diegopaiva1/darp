/**
 * @file    main.cpp
 * @author  Diego Paiva
 * @date    24/09/2019
 */

#include "algorithms/ReactiveGrasp.hpp"
#include "data-structures/Singleton.hpp"
#include "gnuplot/Gnuplot.hpp"
#include "utils/SolutionFileStorer.hpp"

int main(const int argc, char const *argv[])
{
  const int argsGiven     = argc - 1;
  const int minArgsAmount = 1;

  if (argsGiven < minArgsAmount) {
    printf("Error: expected at least %d argument(s) - %d given\n", minArgsAmount, argsGiven);
    return EXIT_FAILURE;
  }

  Singleton *instance = Singleton::getInstance();
  instance->init(argv[1]);

  std::pair<Solution, double> solutionWithElapsedTime = ReactiveGrasp::solve(2500, 250, {
    0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 1.0
  });

  Solution solution  = solutionWithElapsedTime.first;
  double elapsedTime = solutionWithElapsedTime.second;

  Gnuplot::plotSolution(solution);

  if (argv[2])
    SolutionFileStorer::storeSolution(argv[2], solution, elapsedTime);
}
