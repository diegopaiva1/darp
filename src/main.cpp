/**
 * @file    main.cpp
 * @author  Diego Paiva
 * @date    24/09/2019
 */

#include <ilcplex/ilocplex.h>
#include <iostream>

#include "data-structures/Singleton.hpp"
#include "algorithms/Grasp.hpp"

#define MIN_ARGS_AMOUNT 1

bool isFeasible(Solution s)
{
  if (s.routes.size() > instance->vehicles.size())
    return false;

  for (Route *r : s.routes) {
    for (int i = 0; i < r->path.size(); i++) {
      if (r->serviceBeginningTimes[i] > r->path[i]->departureTime) {
        printf("Time window violation at point %d in route %d\n", i, r->vehicle->id);
        return false;
      }

      if (r->path[i]->isPickup() && r->ridingTimes[i] > r->path[i]->maxRideTime) {
        printf("Riding time violation at point %d in route %d\n", i, r->vehicle->id);
        return false;
      }

      if (r->load[i] > r->vehicle->capacity) {
        printf("Vehicle load violation at point %d in route %d\n", i, r->vehicle->id);
        return false;
      }

      // if (r->batteryLevels[i] < 0) {
      //   printf("Battery level violation at point %d in route %d\n", i, r->vehicle->id);
      //   return false;
      // }

      // if (r->path[i]->isStation() && r->load[i] != 0) {
      //   printf("Load violation at station %d in route %d\n", i, r->vehicle->id);
      //   return false;
      // }
    }
  }

  return true;
}

int main(int argc, char *argv[])
{
  int argsGiven = argc - 1;

  if (argsGiven < MIN_ARGS_AMOUNT) {
    printf("Error: expected at least %d argument(s) - %d given\n", MIN_ARGS_AMOUNT, argsGiven);
    return EXIT_FAILURE;
  }

  instance->init(argv[1]);

  std::vector<Solution> solutions;
  Solution best;

  for (int i = 0; i < 10000; i++) {
    Solution s = Grasp::solve(1, 1, {1.0});

    if (s.isFeasible()) {
      if (solutions.size() == 0 || s.cost < best.cost)
        best = s;

      solutions.push_back(s);
    }
  }

  printf("Total de soluções viáveis: %d\n", solutions.size());
  printf("Melhor solução: %f\n", best.cost);

  try {
    // Initialize the environment
    IloEnv env;
    IloModel model(env);
    IloCplex cplex(model);

    // Decision variables
    int totalRoutes = 0;

    for (Solution &s : solutions)
      for (Route *r : s.routes)
        totalRoutes++;

    IloNumVarArray x(env, totalRoutes, 0, 1, ILOBOOL);

    // Objective Function
    IloExpr expr1(env);

    for (int i = 0; i < solutions.size(); i++)
      expr1 += solutions[i].cost * x[i];

    IloObjective obj = IloMinimize(env, expr1);
    model.add(obj);
    expr1.end();

    IloExpr expr2(env);

    // Constraints
    for (int i = 0; i < totalRoutes; i++)
      expr2 += x[i];

    model.add(expr2 == 1);

    cplex.extract(model);
    cplex.solve();

    std::cout << "Objective function: " << cplex.getObjValue() << '\n';

    expr2.end();
    cplex.end();
    model.end();
    env.end();
  }
  catch(IloException& e) {
    std::cerr << "IloException: " << e << '\n';
  }

  return 0;
}
