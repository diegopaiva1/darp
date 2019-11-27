/**
 * @file    main.cpp
 * @author  Diego Paiva
 * @date    24/09/2019
 */

#include <iostream>
#include <iomanip>

#include "data-structures/Singleton.hpp"
#include "algorithms/Grasp.hpp"
#include "gnuplot/Gnuplot.hpp"
#include "utils/Timer.hpp"

#define MIN_ARGS_AMOUNT 1

void storeResults(std::string fileName, Solution s, double cpuTime);
bool isEmpty(std::fstream& file);

int main(int argc, char *argv[])
{
  int argsGiven = argc - 1;

  if (argsGiven < MIN_ARGS_AMOUNT) {
    printf("Error: expected at least %d argument(s) - %d given\n", MIN_ARGS_AMOUNT, argsGiven);
    return EXIT_FAILURE;
  }

  Singleton *instance = Singleton::getInstance();
  instance->init(argv[1]);

  Timer timer;
  Solution solution = Grasp::solve(100, 10, {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0});
  double elapsed = timer.elapsedInMinutes();

  for (auto k = solution.routes.begin(); k != solution.routes.end(); )
    if (k->path.size() <= 2)
      k = solution.routes.erase(k);
    else
      k++;

  float tt  = 0.0;
  float ert = 0.0;

  for (Route route : solution.routes) {
    route.printPath();
    printf("\n");
    route.printSchedule();
    printf("\n");

    for (int i = 0; i < route.path.size(); i++) {
      if (i < route.path.size() - 1)
        tt += instance->getTravelTime(route.path[i], route.path[i + 1]);

      if (route.path[i]->isPickup()) {
        printf("\n (%d, %d) travel time = %.2f, ideal travel time = %.2f\n", route.path[i]->id, route.path[i]->id + instance->requestsAmount, route.ridingTimes[i], instance->getTravelTime(instance->getRequest(route.path[i]).pickup, instance->getRequest(route.path[i]).delivery));
        float rideTimeExcess = route.ridingTimes[i] - instance->getTravelTime(instance->getRequest(route.path[i]).pickup, instance->getRequest(route.path[i]).delivery);
        ert += rideTimeExcess;
      }

      if (route.serviceBeginningTimes[i] > route.path[i]->departureTime)
        printf("\nViolated time window at point %d in route %d", i, route.vehicle.id);
      else if (route.load[i] > route.vehicle.capacity)
        printf("\nViolated load at point %d in route %d", i, route.vehicle.id);
      else if (route.ridingTimes[i] > route.path[i]->maxRideTime)
        printf("\nViolated ride time at point %d in route %d", i, route.vehicle.id);
      else if (route.batteryLevels[i] < 0.0 || route.batteryLevels[i] > route.vehicle.batteryCapacity)
        printf("\nViolated battery levels at point %d in route %d", i, route.vehicle.id);
      else if (route.batteryLevels[route.path.size() - 1] < route.vehicle.batteryCapacity * route.vehicle.minFinalBatteryRatioLevel)
        printf("\nViolated final battery level at point %d in route %d", i, route.vehicle.id);
    }
  }

  printf("\ntt  = %.2f", tt);
  printf("\nert = %.2f\n", ert);

  if (solution.isFeasible())
    printf("Viável\nCusto = %.2f\nt = %.2fmin\n", solution.cost, elapsed);
  else
    printf("Inviável\nCusto = %.2f [load = %d, timeWindow = %.2f, maxRideTime = %.2f]\nt = %.2fmin\n",
            solution.cost, solution.loadViolation, solution.timeWindowViolation, solution.maxRideTimeViolation, elapsed);

  Gnuplot::plotSolution(solution);

  if (argv[2])
    storeResults(argv[2], solution, elapsed);

  return EXIT_SUCCESS;
}

void storeResults(std::string fileName, Solution s, double elapsed)
{
  std::fstream file(fileName, std::fstream::in | std::fstream::out | std::fstream::app);

  if (isEmpty(file))
    file << "Veiculos;Custo;CPU (s)\n";

  file << s.routes.size() << ";" << std::fixed << std::setprecision(2) << s.cost << ";" << elapsed << "\n";
}

bool isEmpty(std::fstream& file)
{
  file.seekg(0, std::ios::end); // Coloque o ponteiro de leitura no início do arquivo
  return file.tellg() == 0;
}
