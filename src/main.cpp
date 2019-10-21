/**
 * @file    main.cpp
 * @author  Diego Paiva
 * @date    24/09/2019
 */

#include <ilcplex/ilocplex.h>
#include <iostream>

#define MIN_ARGS_AMOUNT 1
#define MAX_INT   std::numeric_limits<int>::max()
#define MAX_FLOAT std::numeric_limits<float>::max()

#include "data-structures/Singleton.hpp"

// Instance will be avaliable to everyone
Singleton *instance = Singleton::getInstance();

#include "algorithms/Grasp.hpp"
#include "utils/Timer.hpp"

int main(int argc, char *argv[])
{
  int argsGiven = argc - 1;

  if (argsGiven < MIN_ARGS_AMOUNT) {
    printf("Error: expected at least %d argument(s) - %d given\n", MIN_ARGS_AMOUNT, argsGiven);
    return EXIT_FAILURE;
  }

  instance->init(argv[1]);

  Solution best;
  std::vector<Solution> solutions;
  std::vector<Route *> routes;

  // Start counting
  Timer timer;

  for (int i = 0; i < 30000; i++) {
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

    std::cout << "Objective function: " << cplex.getObjValue() << '\n';
    std::cout << "Total elapsed time: " << timer.elapsedInSeconds() << "s" << '\n';

    cplex.end();
    model.end();
    env.end();
  }
  catch(IloException& e) {
    std::cerr << "IloException: " << e << '\n';
  }

  return EXIT_SUCCESS;
}
