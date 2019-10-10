/**
 * @file    Route.hpp
 * @author  Diego Paiva
 * @date    06/10/2019
 */

#ifndef ROUTE_H_INCLUDED
#define ROUTE_H_INCLUDED

#include "Vehicle.hpp"
#include "Request.hpp"

#include <limits>
#include <algorithm>

class Route
{
public:
  Vehicle *vehicle;
  std::vector<Node *> path;
  std::vector<int>    load;
  std::vector<float>  arrivalTimes;
  std::vector<float>  serviceBeginningTimes;
  std::vector<float>  departureTimes;
  std::vector<float>  waitingTimes;
  std::vector<float>  ridingTimes;
  std::vector<float>  batteryLevels;

  Route(Vehicle *vehicle) { this->vehicle = vehicle; }

  Route() {}

  ~Route() {}

  float getTravelTime(Node *n1, Node *n2) { return n1->point->getDistanceFrom(n2->point); }

  // float duration() { return serviceBeginningTimes.back() - serviceBeginningTimes.front(); }

  // void updateArrivalTime(int index)
  // {
  //   if (index == 0)
  //     arrivalTimes[index] = 0;
  //   else
  //     arrivalTimes[index] = departureTimes[index - 1] + getTravelTime(path[index - 1], path[index]);
  // }

  // void updateServiceBeginningTime(int index)
  // {
  //   if (index == 0)
  //     serviceBeginningTimes[index] = departureTimes[index];
  //   else
  //     serviceBeginningTimes[index] = std::max(path[index]->departureTime, arrivalTimes[index]);
  // }

  // void updateWaitingTime(int index)
  // {
  //   if (index == 0)
  //     waitingTimes[index] = 0;
  //   else
  //     waitingTimes[index] = serviceBeginningTimes[index] - arrivalTimes[index];
  // }

  // void updateDepartureTime(int index)
  // {
  //   if (index == 0)
  //     departureTimes[index] = serviceBeginningTimes[index];
  //   else if (index == path.size() - 1)
  //     departureTimes[index] = 0;
  //   else if (path[index]->type == Type::PICKUP || path[index]->type == Type::DELIVERY)
  //     departureTimes[index] = serviceBeginningTimes[index] + path[index]->serviceTime;
  //   else
  //     // Na parada intermediária em um BSS considera-se que se leva 3 minutos para trocar a bateria
  //     departureTimes[index] = serviceBeginningTimes[index] + 3;
  // }

  // void updateRidingTime(int index, int n)
  // {
  //   if (path[index]->type != Type::PICKUP)
  //     throw "Não é possível atualizar o tempo de viagem deste nó: não é um ponto de embarque";

  //   ridingTimes[index] = serviceBeginningTimes[getDeliveryIndexOf(index, n)] - departureTimes[index];
  // }

  // void updateStaffSeatsLoad(int index)
  // {
  //   if (index == 0 || index == path.size() - 1)
  //     staffSeatsLoad[index] = 0;
  //   else
  //     staffSeatsLoad[index] = staffSeatsLoad[index - 1] + path[index]->staffSeats;
  // }

  // void updatePatientSeatsLoad(int index)
  // {
  //   if (index == 0 || index == path.size() - 1)
  //     patientSeatsLoad[index] = 0;
  //   else
  //     patientSeatsLoad[index] = patientSeatsLoad[index - 1] + path[index]->patientSeats;
  // }

  // void updateWheelchairsLoad(int index)
  // {
  //   if (index == 0 || index == path.size() - 1)
  //     wheelchairsLoad[index] = 0;
  //   else
  //     wheelchairsLoad[index] = wheelchairsLoad[index - 1] + path[index]->wheelchairs;
  // }

  // void updateStretchersLoad(int index)
  // {
  //   if (index == 0 || index == path.size() - 1)
  //     stretchersLoad[index] = 0;
  //   else
  //     stretchersLoad[index] = stretchersLoad[index - 1] + path[index]->stretchers;
  // }

  // void updateBatteryLevels(int i)
  // {
  //   // int totalUsers = staffSeatsLoad[i] + patientSeatsLoad[i] +
  //   //                  wheelchairsLoad[i] + stretchersLoad[i];

  //   // float totalMass = VEHICLE_MASS_KG + AVERAGE_USER_WEIGHT_KG * (totalUsers);

  //   // float tractivePower = (
  //   //   (AIR_MASS_DENSITY_KG_PER_M3/2.0) * VEHICLE_FRONTAL_SURFACE_AREA_M2 * AERODYNAMIC_DRAG_COEFFICIENT *
  //   //   pow(VEHICLE_SPEED_KM_H, 2) + totalMass * ACCELERATION_M_PER_S2 +
  //   //   MASS_CORRECTION_FACTOR * VEHICLE_MASS_KG * ACCELERATION_M_PER_S2 + totalMass * GRAVITY_M_PER_S2 *
  //   //   ( sin(roadSlopeAngle(i, i + 1) * PI/180) + ROLLING_RESISTANCE_COEFFICIENT * cos(roadSlopeAngle(i, i + 1) * PI/180) )
  //   // ) * VEHICLE_SPEED_KM_H;

  //   // float mechanicalPower;

  //   // if (tractivePower < 0)
  //   //   mechanicalPower = tractivePower * GEAR_EFFICIENCY_COEFFICIENT;
  //   // else if (tractivePower > 0)
  //   //   mechanicalPower = tractivePower / GEAR_EFFICIENCY_COEFFICIENT;

  //   // float inputPower;

  //   // float x = 0.001 * std::abs(mechanicalPower)/(RATE_POWER_KW);

  //   // float generatorEfficiency;
  //   // float motorEfficiency;

  //   // if (x >= 0.0 && x < 0.25)
  //   //   generatorEfficiency = (0.925473 * x + 0.000148) * ( 1 / (x + 0.014849) );
  //   // else if (x >= 0.25 && x < 0.75)
  //   //   generatorEfficiency = 0.075312 * x + 0.858605;
  //   // else if (x >= 0.75)
  //   //   generatorEfficiency = -0.062602 * x + 0.971034;

  //   // if (x >= 0.0 && x < 0.25)
  //   //   motorEfficiency = (0.924300 * x + 0.000127) * ( 1 / (x + 0.012730) );
  //   // else if (x >= 0.25 && x < 0.75)
  //   //   motorEfficiency = 0.080000 * x + 0.860000;
  //   // else if (x >= 0.75)
  //   //   motorEfficiency = -0.073600 * x + 0.975200;

  //   // if (tractivePower < 0)
  //   //   inputPower = mechanicalPower * generatorEfficiency * MOTOR_EFFICIENCY_NORMALIZATION_FACTOR;
  //   // else if (tractivePower > 0)
  //   //   inputPower = mechanicalPower / (motorEfficiency * MOTOR_EFFICIENCY_NORMALIZATION_FACTOR);

  //   // float outputPower = inputPower + ACCESSORIES_POWER_CONSUMPTION_W;

  //   // float energyConsumption;

  //   // // Equação 1.a
  //   // if (tractivePower < 0 && outputPower < 0)
  //   // {
  //   //   energyConsumption = (tractivePower * GEAR_EFFICIENCY_COEFFICIENT * generatorEfficiency *
  //   //                       ( (0.001 * mechanicalPower) / (RATE_POWER_KW)) * MOTOR_EFFICIENCY_NORMALIZATION_FACTOR +
  //   //                       ACCESSORIES_POWER_CONSUMPTION_W) * (sqrt(ROUND_TRIP_EFFICIENCY) / 60.0);
  //   // }
  //   // // Equação 1.b
  //   // else if (tractivePower < 0 && outputPower > 0)
  //   // {
  //   //   energyConsumption = (tractivePower * GEAR_EFFICIENCY_COEFFICIENT * generatorEfficiency *
  //   //                       ( (0.001 * mechanicalPower) / (RATE_POWER_KW)) * MOTOR_EFFICIENCY_NORMALIZATION_FACTOR +
  //   //                       ACCESSORIES_POWER_CONSUMPTION_W) * ( (1.0/60.0) / sqrt(ROUND_TRIP_EFFICIENCY));
  //   // }
  //   // // Equação 1.c
  //   // else if (tractivePower > 0)
  //   // {
  //   //   energyConsumption = (
  //   //     ( tractivePower / (GEAR_EFFICIENCY_COEFFICIENT * generatorEfficiency *
  //   //                       ( (0.001 * mechanicalPower) / (RATE_POWER_KW)) * MOTOR_EFFICIENCY_NORMALIZATION_FACTOR)
  //   //     ) + ACCESSORIES_POWER_CONSUMPTION_W * ( (1.0/60.0) * sqrt (ROUND_TRIP_EFFICIENCY) )
  //   //   );
  //   // }

  //   // float totalEnergyConsumptionInKw = (energyConsumption * getTravelTime(path[i - 1], path[i]))/1000;

  //   if (path[i]->type == Type::BSS || path[i]->type == Type::DEPOT)
  //     batteryLevels[i] = BATTERY_CAPACITY_KW;
  //   else
  //     batteryLevels[i] = batteryLevels[i - 1] - 2 * path[i - 1]->point->getDistanceFrom(path[i]->point);
  // }

  // float roadSlopeAngle(int i, int j)
  // {
  //   return atan((path[j]->gradient - path[i]->gradient) / 100.0) * 180/PI;
  // }

  float getTotalDistance()
  {
    float distance = 0.0;

    for (int i = 0; i < path.size() - 1; i++)
      distance += path[i]->point->getDistanceFrom(path[i + 1]->point);

    return distance;
  }

  // // Retorna o índice 'i' de desembarque (delivery) de um nó 'j' de embarque (pickup) da rota
  // int getDeliveryIndexOf(int j, int n)
  // {
  //   if (path[j]->type != Type::PICKUP)
  //     throw "O nó fornecido não é um ponto de embarque";

  //   for (int i = 1; i < path.size(); i++)
  //     if (path[i]->id == path[j]->id + n)
  //       return i;
  // }

  // // Retorna o índice 'i' de embarque (pickup) de um nó 'j' de desembarque (delivery) da rota
  // int getPickupIndexOf(int j, int n)
  // {
  //   if (path[j]->type != Type::DELIVERY)
  //     throw "O nó fornecido não é um ponto de desembarque";

  //   for (int i = 1; i < path.size(); i++)
  //     if (path[i]->id == path[j]->id - n)
  //       return i;
  // }

  // void setSchedule(int n)
  // {
  //   resizeVectors();

  //   departureTimes[0] = path[0]->departureTime;

  //   for (int i = 0; i < path.size(); i++) {
  //     updateArrivalTime(i);
  //     updateServiceBeginningTime(i);

  //     if (serviceBeginningTimes[i] > path[i]->arrivalTime) return;

  //     updateWaitingTime(i);
  //     updateDepartureTime(i);

  //     updateWheelchairsLoad(i);

  //     if (wheelchairsLoad[i] > vehicle->wheelchairs) return;

  //     updateStretchersLoad(i);

  //     if (stretchersLoad[i] > vehicle->stretchers) return;

  //     updatePatientSeatsLoad(i);

  //     if (patientSeatsLoad[i] > vehicle->patientSeats) {
  //       patientSeatsLoad[i] -= path[i]->patientSeats;
  //       stretchersLoad[i] = stretchersLoad[i - 1] + path[i]->patientSeats;

  //       if (stretchersLoad[i] > vehicle->stretchers)
  //         return;
  //     }

  //     updateStaffSeatsLoad(i);

  //     if (staffSeatsLoad[i] > vehicle->staffSeats) {
  //       return;
  //     }

  //     updateBatteryLevels(i);
  //   }

  //   float f0 = calculateForwardSlackTime(0, n);

  //   float waitingTimeSum = 0.0;
  //   for (int i = 1; i < path.size() - 1; i++)
  //     waitingTimeSum += waitingTimes[i];

  //   departureTimes[0] = path[0]->departureTime + std::min(f0, waitingTimeSum);

  //   for (int i = 0; i < path.size(); i++) {
  //     updateArrivalTime(i);
  //     updateServiceBeginningTime(i);
  //     updateWaitingTime(i);
  //     updateDepartureTime(i);
  //   }

  //   bool allRidingTimesRespected = true;

  //   for (int i = 1; i < path.size() - 1; i++) {
  //     if (path[i]->type == Type::PICKUP) {
  //       updateRidingTime(i, n);

  //       if (ridingTimes[i] > path[i]->maxUserRideTime)
  //         allRidingTimesRespected = false;
  //     }
  //   }

  //   if (allRidingTimesRespected)
  //     return;

    // for (int i = 1; i < path.size() - 1; i++) {
    //   if (path[i]->type == Type::PICKUP) {
    //     float forwardSlackTime = calculateForwardSlackTime(i, n);

    //     float waitingTimeSum = 0.0;
    //     for (int p = i + 1; p < path.size() - 1; p++)
    //       waitingTimeSum += waitingTimes[p];

    //     waitingTimes[i] += std::min(forwardSlackTime, waitingTimeSum);
    //     serviceBeginningTimes[i] = arrivalTimes[i] + waitingTimes[i];
    //     departureTimes[i] = serviceBeginningTimes[i] + path[i]->serviceTime;

    //     // Atualizamos os vetores para cada nó posterior ao nó do índice i
    //     for (int j = i + 1; j < path.size(); j++) {
    //       updateArrivalTime(j);
    //       updateServiceBeginningTime(j);
    //       updateWaitingTime(j);
    //       updateDepartureTime(j);
    //     }

    //     bool allRidingTimesRespected = true;

    //     for (int k = i + 1; k < path.size() - 1; k++) {
    //       if (path[k]->type == Type::DELIVERY) {
    //         int pickupIndex = getPickupIndexOf(k, n);

    //         updateRidingTime(pickupIndex, n);

    //         if (ridingTimes[pickupIndex] > path[pickupIndex]->maxUserRideTime)
    //           allRidingTimesRespected = false;
    //       }
    //     }

    //     if (allRidingTimesRespected)
    //       return;
    //   }
    // }
  // }

  // /* O forward slack time é calculado como o menor de todos os slack times entre
  //  * o index (ponto da rota que se deseja calcular) e o ponto final da rota
  //  */
  // float calculateForwardSlackTime(int index, int n)
  // {
  //   float forwardSlackTime;

  //   for (int j = index; j < path.size(); j++) {
  //     float waitingTimeSum = 0.0;

  //     for (int p = index + 1; p <= j; p++)
  //       waitingTimeSum += waitingTimes[p];

  //     float userRideTimeWithDeliveryAtJ = 0.0;

  //     if (path[j]->type == Type::DELIVERY)
  //       userRideTimeWithDeliveryAtJ = ridingTimes[getPickupIndexOf(j, n)];

  //     float slackTime;

  //     if (path[index]->type == Type::PICKUP)
  //       slackTime = waitingTimeSum +
  //                   std::max(0.0f, std::min(path[j]->arrivalTime - serviceBeginningTimes[j],
  //                                           path[index]->maxUserRideTime - userRideTimeWithDeliveryAtJ));
  //     else if (index == 0)
  //       slackTime = waitingTimeSum + std::max(0.0f, path[j]->arrivalTime - serviceBeginningTimes[j]);
  //     else
  //       slackTime = 0.0;

  //     if (j == index || slackTime < forwardSlackTime)
  //       forwardSlackTime = slackTime;
  //   }

  //   return forwardSlackTime;
  // }

  // void resizeVectors()
  // {
  //   int size = path.size();
  //   arrivalTimes.clear();
  //   arrivalTimes.resize(size);
  //   serviceBeginningTimes.clear();
  //   serviceBeginningTimes.resize(size);
  //   departureTimes.clear();
  //   departureTimes.resize(size);
  //   waitingTimes.clear();
  //   waitingTimes.resize(size);
  //   ridingTimes.clear();
  //   ridingTimes.resize(size);
  //   staffSeatsLoad.clear();
  //   staffSeatsLoad.resize(size);
  //   patientSeatsLoad.clear();
  //   patientSeatsLoad.resize(size);
  //   wheelchairsLoad.clear();
  //   wheelchairsLoad.resize(size);
  //   stretchersLoad.clear();
  //   stretchersLoad.resize(size);
  //   batteryLevels.clear();
  //   batteryLevels.resize(size);
  // }

  // std::vector<Arc *> getArcs()
  // {
  //   std::vector<Arc *> arcs;

  //   for (int i = 0; i < path.size() - 1; i++)
  //     arcs.push_back(new Arc(path[i], path[i + 1]));

  //   return arcs;
  // }

  // bool isFeasible()
  // {
  //   for (int i = 0; i < path.size(); i++) {
  //     if (serviceBeginningTimes[i] > path[i]->arrivalTime)
  //       return false;

  //     if (path[i]->type == Type::PICKUP && ridingTimes[i] > path[i]->maxUserRideTime)
  //       return false;

  //     if (staffSeatsLoad[i] > vehicle->staffSeats)
  //       return false;

  //     if (patientSeatsLoad[i] > vehicle->patientSeats)
  //       return false;

  //     if (wheelchairsLoad[i] > vehicle->wheelchairs)
  //       return false;

  //     if (stretchersLoad[i] > vehicle->stretchers)
  //       return false;

  //     // if (batteryLevels[i] < 0)
  //     //   return false;
  //   }

  //   float routeDuration = serviceBeginningTimes.back() - serviceBeginningTimes.front();

  //   if (routeDuration > vehicle->maxRouteDuration)
  //     return false;

  //   return true;
  // }

  // bool hasFeasiblyInserted(Request *request, Instance instance)
  // {
  //   Route *bestFeasible = new Route();

  //   float bestCost = std::numeric_limits<float>::max();

  //   bool wasFeasiblyInserted = false;

  //   for (int i = 1; i < path.size() - 2; i++) {
  //     if (i == 1) {
  //       path.insert(path.begin() + 1, request->pickup);
  //       path.insert(path.begin() + 2, request->delivery);
  //     }
  //     else {
  //       std::swap(path[i], path[i - 1]);

  //       for (int j = path.size() - 2; j > i + 1; j--)
  //         std::swap(path[j], path[j - 1]);
  //     }

  //     setSchedule(instance.requests.size());

  //     if (this->isFeasible()) {
  //       float totalDistance = this->getTotalDistance();
  //       float totalTime = this->serviceBeginningTimes.back() - this->serviceBeginningTimes.front();

  //       float cost = ALPHA1 * totalDistance + ALPHA2 * totalTime;

  //       if (cost < bestCost) {
  //         bestCost = cost;
  //         bestFeasible->path = path;
  //       }

  //       wasFeasiblyInserted = true;
  //     }

  //     for (int j = i + 1; j < path.size() - 2; j++) {
  //       std::swap(path[j], path[j + 1]);

  //       setSchedule(instance.requests.size());

  //       if (this->isFeasible()) {
  //         float totalDistance = this->getTotalDistance();
  //         float totalTime = this->serviceBeginningTimes.back() - this->serviceBeginningTimes.front();

  //         float cost = ALPHA1 * totalDistance + ALPHA2 * totalTime;

  //         if (cost < bestCost) {
  //           bestCost = cost;
  //           bestFeasible->path = path;
  //         }

  //         wasFeasiblyInserted = true;
  //       }
  //     }
  //   }

  //   if (wasFeasiblyInserted) {
  //     path = bestFeasible->path;
  //   }
  //   else {
  //     path.erase(std::remove(path.begin(), path.end(), request->pickup), path.end());
  //     path.erase(std::remove(path.begin(), path.end(), request->delivery), path.end());
  //   }

  //   delete bestFeasible;
  //   bestFeasible = NULL;

  //   return wasFeasiblyInserted;
  // }

  void printPath()
  {
    for (Node *node : path)
      printf("%d ", node->id);
  }

  void printSchedule()
  {
    for (int i = 0; i < path.size(); i++) {
      printf("A[%02d] = %6.4g\t", i,  arrivalTimes[i]);
      printf("B[%02d] = %6.4g\t", i,  serviceBeginningTimes[i]);
      printf("D[%02d] = %6.4g\t", i,  departureTimes[i]);
      printf("W[%02d] = %.4g\t",  i,  waitingTimes[i]);
      printf("R[%02d] = %.4g\t",  i,  ridingTimes[i]);
      printf("Z[%02d] = %.4g\t",  i,  batteryLevels[i]);
      printf("Q0[%02d] = %d  ",   i,  load[i]);
      printf("\n");
    }
  }
};

#endif // ROUTE_H_INCLUDED
