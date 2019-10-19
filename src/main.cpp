/**
 * @file    main.cpp
 * @author  Diego Paiva
 * @date    24/09/2019
 */

#include <iostream>

#include "data-structures/Singleton.hpp"
#include "algorithms/Grasp.hpp"

#define MIN_ARGS_AMOUNT 1

bool isFeasible(Solution s)
{
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

      if (r->batteryLevels[i] < 0) {
        printf("Battery level violation at point %d in route %d\n", i, r->vehicle->id);
        return false;
      }

      if (r->path[i]->isStation() && r->load[i] != 0) {
        printf("Load violation at station %d in route %d\n", i, r->vehicle->id);
        return false;
      }
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

  Singleton *instance = Singleton::getInstance();
  instance->init(argv[1]);

  Solution solution = Grasp::solve(1000, 100, {0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50});

  for (Route *route : solution.routes) {
    route->printPath();
    route->printSchedule();
    printf("\n");

    // for (int i = 0; i < route->path.size(); i++) {
    //   if (route->batteryLevels[i] < 0) {
    //     struct Stop {
    //       int   position;
    //       int   stationId;
    //       float maxRechargingTime;
    //       float batteryLevel;
    //       float distance;
    //     };

    //     std::vector<Stop> stops;

    //     for (int s = i - 1; s >= 0; s--) {
    //       if (route->load[s] == 0) {
    //         int nearestStationId = Singleton::getInstance()->nearestStations[s + 1][s + 2];
    //         float maxRechargingTime = route->path[s + 1]->departureTime - route->serviceBeginningTimes[s + 1];
    //         float distance = 0.0;

    //         for (int k = s + 1; route->load[k] != 0 ; k++)
    //           distance += Singleton::getInstance()->getTravelTime(route->path[k], route->path[k + 1]);

    //         stops.push_back({
    //           s+1, nearestStationId, maxRechargingTime,
    //           route->batteryLevels[s + 1] - route->vehicle->dischargingRate * Singleton::getInstance()->travelTimes[nearestStationId][s+2], distance}
    //         );
    //       }
    //     }

    //     int chosenPosition;
    //     float newBatteryLevel;
    //     for (struct Stop p : stops) {
    //       if (p.batteryLevel > 0) {
    //         chosenPosition = p.position;
    //         newBatteryLevel = p.batteryLevel + Singleton::getInstance()->getNode(p.stationId)->rechargingRate * p.maxRechargingTime;
    //         break;
    //       }
    //     }

    //     printf("Posições possíveis da rota para abastecer o ponto %d:\n", i);
    //     for (struct Stop p : stops)
    //       printf("%d: tmax = %f, dist = %f, stationId = %d, z = %f\n", p.position, p.maxRechargingTime, p.distance, p.stationId, p.batteryLevel);
    //     printf("\n");
    //     printf("posição escolhida = %d e novo nivel da bateria = %f\n", chosenPosition, newBatteryLevel);
    //   }
    // }
  }

  if (isFeasible(solution))
    printf("Viável - Custo = %.2f\n", solution.cost);
  else
    printf("Inviável - Custo = %.2f\n", solution.cost);

  return 0;
}
