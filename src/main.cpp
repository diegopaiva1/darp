/**
 * @file    main.cpp
 * @author  Diego Paiva
 * @date    24/09/2019
 */

#include <ilcplex/ilocplex.h>
#include <iostream>

int main(int argc, char *argv[])
{
  try {
    // Initialize the environment
    IloEnv env;
    IloModel model(env);
    IloCplex cplex(model);

    // Decision variables
    IloNumVar x11(env), x12(env), x13(env), x21(env), x22(env), x23(env);

    // Objective Function
    IloObjective obj = IloMinimize(env, 4 * x11 + 2 * x12 + 5 * x13 + 11 * x21 + 7 * x22 + 4 * x23);
    model.add(obj);

    // Constraints
    model.add(x11 + x12 + x13 <=  800);
    model.add(x21 + x22 + x23 <= 1000);
    model.add(x11 + x21       ==  500);
    model.add(x12 + x22       ==  400);
    model.add(x13 + x23       ==  900);

    cplex.extract(model);
    cplex.solve();

    std::cout << "Objective function: " << cplex.getObjValue() << '\n';
    std::cout << "x11:" << cplex.getValue(x11) << '\n';
    std::cout << "x12:" << cplex.getValue(x12) << '\n';
    std::cout << "x13:" << cplex.getValue(x13) << '\n';
    std::cout << "x21:" << cplex.getValue(x21) << '\n';
    std::cout << "x22:" << cplex.getValue(x22) << '\n';
    std::cout << "x23:" << cplex.getValue(x23) << '\n';

    cplex.end();
    model.end();
    env.end();
  }
  catch(IloException& e) {
    std::cerr << "IloException: " << e << '\n';
  }

  return 0;
}
