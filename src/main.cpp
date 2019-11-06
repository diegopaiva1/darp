/**
 * @file    main.cpp
 * @author  Diego Paiva
 * @date    24/09/2019
 */

#include <ilcplex/ilocplex.h>
#include <iostream>
#include <iomanip>

#include "data-structures/Singleton.hpp"
#include "algorithms/Grasp.hpp"
#include "gnuplot/Gnuplot.hpp"
#include "utils/Timer.hpp"

void storeResults(std::string fileName, Solution s, double elapsed, int totalRoutes, float originalCost);
bool isEmpty(std::fstream& file);

#define MIN_ARGS_AMOUNT 1

int main(int argc, char *argv[])
{
  int argsGiven = argc - 1;

  if (argsGiven < MIN_ARGS_AMOUNT) {
    printf("Error: expected at least %d argument(s) - %d given\n", MIN_ARGS_AMOUNT, argsGiven);
    return EXIT_FAILURE;
  }

  Singleton *instance = Singleton::getInstance();
  instance->init(argv[1]);

  Solution best;
  std::vector<Solution> solutions;
  std::vector<Route *> routes;

  // Start counting
  Timer timer;

  for (int i = 0; i < 20000; i++) {
    Solution s = Grasp::solve(1, 1, {1.0});

    if (s.isFeasible()) {
      if (solutions.size() == 0 || s.cost < best.cost)
        best = s;

      for (Route *r : s.routes)
        routes.push_back(r);

      solutions.push_back(s);
    }
  }

  int totalRoutes = routes.size();

  printf("Total de rotas geradas: %d\n", totalRoutes);
  printf("Melhor solução: %f\n", best.cost);

  try {
    // Initialize the environment
    IloEnv env;
    IloModel model(env);
    IloCplex cplex(model);

    IloNumVarArray selected(env, totalRoutes, 0, 1, ILOBOOL);

    // Objective Function
    IloExpr expr(env);

    for (int i = 0; i < totalRoutes; i++)
      expr += routes[i]->cost * selected[i];

    IloObjective obj = IloMinimize(env, expr);
    model.add(obj);
    expr.end();

    // Constraints
    for (int j = 0; j < instance->requestsAmount; j++) {
      IloExpr expr(env);

      for (int i = 0; i < totalRoutes; i++)
        expr += routes[i]->hasRequest(instance->requests[j]) * selected[i];

      model.add(expr == 1);
      expr.end();
    }

    cplex.extract(model);
    cplex.solve();

    double elapsed = timer.elapsedInSeconds();

    std::cout << "Objective function: " << cplex.getObjValue() << '\n';
    std::cout << "Total elapsed time: " << elapsed << "s" << '\n';

    Solution solutionFound;

    for (int i = 0; i < totalRoutes; i++)
    {
      if (cplex.getValue(selected[i]) == 1) {
        solutionFound.routes.push_back(routes[i]);
        solutionFound.cost += routes[i]->cost;
      }
    }

    cplex.end();
    model.end();
    env.end();

    for (Route *r : solutionFound.routes) {
      printf("\n");
      r->printPath();
      printf("\n");
      r->printSchedule();
      printf("\n");
    }

    // If second argument was given we store the results in the output file
    if (argv[2])
      storeResults(argv[2], solutionFound, elapsed, totalRoutes, best.cost);
  }
  catch(IloException& e) {
    std::cerr << "IloException: " << e << '\n';
  }

  return EXIT_SUCCESS;
}

void storeResults(std::string fileName, Solution s, double elapsed, int totalRoutes, float originalCost)
{
  std::fstream file(fileName, std::fstream::in | std::fstream::out | std::fstream::app);

  if (isEmpty(file))
    file << "Rotas geradas;Heurística;CPLEX;CPU (s)\n";

  file << totalRoutes << ";" << std::fixed << std::setprecision(2)  << originalCost << ";" << s.cost << ";" << elapsed << "\n";
}

bool isEmpty(std::fstream& file)
{
  file.seekg(0, std::ios::end); // Coloque o ponteiro de leitura no início do arquivo
  return file.tellg() == 0;
}
