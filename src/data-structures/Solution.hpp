/**
 * @file   Solution.hpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#ifndef SOLUTION_H_INCLUDED
#define SOLUTION_H_INCLUDED

#include "Route.hpp"

class Solution
{
public:
  std::vector<Route *> routes;

  Solution() { }

  ~Solution() { }

  float cost()
  {
    float cost = 0.0;

    for (Route *route : routes)
      cost += route->getTotalDistance();

    return cost;
  }

  // float evaluate(float &alpha, float &beta, float &gamma, float &tau, float &delta)
  // {
  //   float routeDurationViolation    = 0.0;
  //   float timeWindowViolation       = 0.0;
  //   float maximumRideTimeViolation  = 0.0;
  //   float patientSeatsLoadViolation = 0.0;
  //   float staffSeatsLoadViolation   = 0.0;
  //   float wheelchairsLoadViolation  = 0.0;
  //   float stretchersLoadViolation   = 0.0;

  //   for (Route *route : routes) {
  //     routeDurationViolation += std::max(0.0f, route->duration() - route->vehicle->maxRouteDuration);

  //     for (int i = 0; i < route->path.size(); i++) {
  //       Node *node = route->path[i];

  //       if (node->type == Type::PICKUP)
  //         maximumRideTimeViolation += std::max(0.0f, route->ridingTimes[i] - node->maxUserRideTime);

  //       if (node->type == Type::PICKUP || node->type == Type::DELIVERY) {
  //         timeWindowViolation += std::max(0.0f, route->serviceBeginningTimes[i] - node->arrivalTime);
  //         patientSeatsLoadViolation += std::max(0, route->patientSeatsLoad[i] - route->vehicle->patientSeats);
  //         staffSeatsLoadViolation += std::max(0, route->staffSeatsLoad[i] - route->vehicle->staffSeats);
  //         wheelchairsLoadViolation += std::max(0, route->wheelchairsLoad[i] - route->vehicle->wheelchairs);
  //         stretchersLoadViolation += std::max(0, route->stretchersLoad[i] - route->vehicle->stretchers);
  //       }
  //     }
  //   }

  //   float loadViolation = patientSeatsLoadViolation + staffSeatsLoadViolation +
  //                         wheelchairsLoadViolation  + stretchersLoadViolation;

    // float factor = 1 + delta;

    // // Ajusta os parâmetros de penalidade para cada restrição
    // loadViolation            == 0 ? alpha /= factor : alpha *= factor;
    // routeDurationViolation   == 0 ? beta  /= factor : beta  *= factor;
    // timeWindowViolation      == 0 ? gamma /= factor : gamma *= factor;
    // maximumRideTimeViolation == 0 ? tau   /= factor : tau   *= factor;

  //   // TODO: Adicionar a violação do nível da bateria na função
  //   return cost() + alpha * (loadViolation) + beta * (routeDurationViolation) +
  //                   gamma * (timeWindowViolation) + tau * (maximumRideTimeViolation);
  // }
};

#endif // SOLUTION_H_INCLUDED
