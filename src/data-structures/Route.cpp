/**
 * @file    Route.hpp
 * @author  Diego Paiva
 * @date    06/10/2019
 */

#include "Route.hpp"
#include "Singleton.hpp"

Singleton *inst = Singleton::getInstance();

Route::Route(Vehicle v)
{
  vehicle = v;
}

Route::Route()
{
  // Empty constructor
}

Route::~Route()
{
  // Empty destructor
}

bool Route::isFeasible()
{
  return loadViolation         == 0 &&
         maxRideTimeViolation  == 0 &&
         timeWindowViolation   == 0 &&
         finalBatteryViolation == 0 &&
         !batteryLevelViolation;
}

void Route::printPath()
{
  printf("Route %d: ", vehicle.id);

  for (Node *node : path)
    printf("%d ", node->id);
}

void Route::printSchedule()
{
  for (int i = 0; i < path.size(); i++) {
    printf("%2d\t", path[i]->id);
    printf("A[%02d] = %6.4g\t", i,  arrivalTimes[i]);
    printf("B[%02d] = %6.4g\t", i,  serviceBeginningTimes[i]);
    printf("W[%02d] = %.4g\t",  i,  waitingTimes[i]);
    printf("D[%02d] = %6.4g\t", i,  departureTimes[i]);
    printf("L[%02d] = %.4g\t",  i,  rideTimes[i]);
    printf("Z[%02d] = %.4g\t",  i,  batteryLevels[i]);
    printf("E[%02d] = %.4g\t",  i,  chargingTimes[i]);
    printf("Q[%02d] = %d\t",    i,  load[i]);
    printf("R[%02d] = %.4g",    i,  rideTimeExcesses[i]);
    printf("\n");
  }
}

void Route::performEightStepEvaluationScheme()
{
  float forwardTimeSlackAtBeginning;
  float waitingTimeSum;

  int size = path.size();
  arrivalTimes.clear();
  arrivalTimes.resize(size);
  serviceBeginningTimes.clear();
  serviceBeginningTimes.resize(size);
  departureTimes.clear();
  departureTimes.resize(size);
  waitingTimes.clear();
  waitingTimes.resize(size);
  rideTimes.clear();
  rideTimes.resize(size);
  load.clear();
  load.resize(size);
  batteryLevels.clear();
  batteryLevels.resize(size);
  chargingTimes.clear();
  chargingTimes.resize(size);
  rideTimeExcesses.clear();
  rideTimeExcesses.resize(size);

  STEP1:
    // printf("Step 1: Set D_o = e_0 = %.2f\n", path[0]->arrivalTime);
    departureTimes[0] = path[0]->arrivalTime;
    serviceBeginningTimes[0] = departureTimes[0];
    batteryLevels[0] = vehicle.initialBatteryLevel;

  // printf("Step 2: Compute A_i, B_i, W_i, D_i, Q_i and Z_i for every vertex i in the route\n");
  STEP2:
    for (int i = 1; i < path.size(); i++) {
      computeLoad(i);

      // Violated vehicle capacity, that's an irreparable violation
      if (load[i] > vehicle.capacity)
        goto STEP8;

      computeArrivalTime(i);
      computeServiceBeginningTime(i);

      // Violated time windows, that's an irreparable violation
      if (serviceBeginningTimes[i] > path[i]->departureTime)
        goto STEP8;

      computeWaitingTime(i);
      computeChargingTime(i);
      computeBatteryLevel(i);

      // Violated battery level, that's an irreparable violation
      if (batteryLevels[i] < 0.0 || batteryLevels[i] > vehicle.batteryCapacity)
        goto STEP8;

      computeDepartureTime(i);
    }

  // printf("\n");
  // printSchedule();
  // printf("\n");

  STEP3:
    // printf("Step 3: Compute F_0\n");
    forwardTimeSlackAtBeginning = computeForwardTimeSlack(0);

  STEP4:
    waitingTimeSum = 0.0;

    for (int p = 1; p < path.size() - 1; p++)
      waitingTimeSum += waitingTimes[p];

    departureTimes[0] = path[0]->arrivalTime + std::min(forwardTimeSlackAtBeginning, waitingTimeSum);
    serviceBeginningTimes[0] = departureTimes[0];
    // printf("Step 4: Set D_o = e_0 + min\{F_0, sum_W_p\} = %.2f + min\{%.2f, %.2f\} = %.2f\n", path[0]->arrivalTime,
    //        forwardTimeSlackAtBeginning, waitingTimeSum, departureTimes[0]);

  // printf("Step 5: Update A_i, B_i, W_i and D_i for every vertex i in the route\n");
  STEP5:
    for (int i = 1; i < path.size(); i++) {
      computeArrivalTime(i);
      computeServiceBeginningTime(i);
      computeWaitingTime(i);
      computeChargingTime(i);
      computeDepartureTime(i);
    }

  // printf("\n");
  // printSchedule();
  // printf("\n");

  // printf("Step 6: Compute L_i for every user i in the route\n");
  STEP6:
    for (int i = 1; i < path.size() - 1; i++)
      if (path[i]->isPickup())
        computeRideTime(i);

  // printf("\n");
  // printSchedule();
  // printf("\n");

  // printf("Step 7: For every vertex j that is an origin\n");
  STEP7:
    for (int j = 1; j < path.size() - 1; j++) {
      if (path[j]->isPickup() && load[j] == 1) {
        STEP7a:
          // printf("\n\tStep 7a: Compute F_%d\n", j);
          float forwardTimeSlack = computeForwardTimeSlack(j);

        STEP7b:
          waitingTimeSum = 0.0;

          for (int p = j + 1; p < path.size() - 1; p++)
            waitingTimeSum += waitingTimes[p];

          // if (load[j] > 1)
          //   forwardTimeSlack = 0;

          float old = waitingTimes[j];
          waitingTimes[j] += std::min(forwardTimeSlack, waitingTimeSum);
          // printf("\n\tStep 7b: Set W_%d = W_%d + min\{F_%d, W_p\} = %.2f + min\{%.2f, %.2f\} = %.2f\n", j, j, j, old, forwardTimeSlack, waitingTimeSum, waitingTimes[j]);
          serviceBeginningTimes[j] = arrivalTimes[j] + waitingTimes[j];
          // printf("\n\tStep 7b: Set B_%d = A_%d + W_%d = %.2f + %.2f = %.2f\n", j, j, j, arrivalTimes[j], waitingTimes[j], serviceBeginningTimes[j]);
          departureTimes[j] = serviceBeginningTimes[j] + path[j]->serviceTime;
          // printf("\n\tStep 7b: Set D_%d = B_%d + d_%d = %.2f + %.2f = %.2f\n", j, j, j, serviceBeginningTimes[j], path[j]->serviceTime, departureTimes[j]);

        // printf("\n\tStep 7c: Update A_i, B_i, W_i and D_i for every vertex i that comes after %d\n", j);
        STEP7c:
          for (int i = j + 1; i < path.size(); i++) {
            computeArrivalTime(i);
            computeServiceBeginningTime(i);
            computeWaitingTime(i);
            computeChargingTime(i);
            computeDepartureTime(i);
          }

        // printSchedule();
        // printf("\n");

        // printf("\n\tStep 7d: Update L_i for each vertex i whose destination lies after %d\n", j);
        STEP7d:
          for (int i = j + 1; i < path.size() - 1; i++)
            if (path[i]->isDelivery())
              computeRideTime(getPickupIndexOf(i));

        // printSchedule();
        // printf("\n");
      }
    }

  for (int i = 0; i < path.size(); i++)
    if (path[i]->isPickup())
      computeRideTimeExcess(i);

  STEP8:
    cost                  = 0.0;
    loadViolation         = 0;
    timeWindowViolation   = 0.0;
    maxRideTimeViolation  = 0.0;
    finalBatteryViolation = 0.0;
    batteryLevelViolation = false;

    for (int i = 0; i < path.size(); i++) {
      if (i < path.size() - 1)
        cost += 0.75 * inst->getTravelTime(path[i], path[i + 1]);

      cost += 0.25 * rideTimeExcesses[i];

      if (path[i]->isPickup())
        maxRideTimeViolation += std::max(0.0f, rideTimes[i] - path[i]->maxRideTime);

      loadViolation += std::max(0, load[i] - vehicle.capacity);
      timeWindowViolation += std::max(0.0f, serviceBeginningTimes[i] - path[i]->departureTime);

      if (batteryLevels[i] < 0 || batteryLevels[i] > vehicle.batteryCapacity)
        batteryLevelViolation = true;
    }

    finalBatteryViolation = std::max(0.0f, vehicle.batteryCapacity * vehicle.minFinalBatteryRatioLevel - batteryLevels[path.size() - 1]);
}

float Route::computeForwardTimeSlack(int i)
{
  float forwardTimeSlack = MAXFLOAT;

  // printf("\tF_%d = min\{", i);
  for (int j = i; j < path.size(); j++) {
    float waitingTimeSum = 0.0;
    float pj = 0.0;

    for (int p = i + 1; p <= j; p++)
      waitingTimeSum += waitingTimes[p];

    if (path[j]->isDelivery() && getPickupIndexOf(j) < i)
      pj = rideTimes[getPickupIndexOf(j)];

    float timeSlack = waitingTimeSum + std::max(0.0f, std::min(path[j]->departureTime - serviceBeginningTimes[j], 30.0f - pj));

    // if (j == path.size() - 1)
    //   printf("%.2f\}", timeSlack);
    // else
    //   printf("%.2f, ", timeSlack);

    if (timeSlack < forwardTimeSlack)
      forwardTimeSlack = timeSlack;
  }

  // printf(" = %.2f\n", forwardTimeSlack);

  return forwardTimeSlack;
}

void Route::computeLoad(int i)
{
  load[i] = load[i - 1] + path[i]->load;
}

void Route::computeArrivalTime(int i)
{
  arrivalTimes[i] = departureTimes[i - 1] + inst->getTravelTime(path[i - 1], path[i]);
}

void Route::computeServiceBeginningTime(int i)
{
  serviceBeginningTimes[i] = std::max(arrivalTimes[i], path[i]->arrivalTime);
}

void Route::computeWaitingTime(int i)
{
  waitingTimes[i] = serviceBeginningTimes[i] - arrivalTimes[i];
}

void Route::computeDepartureTime(int i)
{
  if (path[i]->isStation())
    departureTimes[i] = serviceBeginningTimes[i] + chargingTimes[i];
  else
    departureTimes[i] = serviceBeginningTimes[i] + path[i]->serviceTime;
}

void Route::computeRideTime(int i)
{
  rideTimes[i] = serviceBeginningTimes[getDeliveryIndexOf(i)] - departureTimes[i];
}

void Route::computeRideTimeExcess(int i)
{
  rideTimeExcesses[i] = rideTimes[i] - inst->getTravelTime(inst->getRequest(path[i]).pickup, inst->getRequest(path[i]).delivery);
}

void Route::computeChargingTime(int i)
{
  if (!path[i]->isStation())
    chargingTimes[i] = 0.0;
  else
    chargingTimes[i] = computeForwardTimeSlack(i + 1);
}

void Route::computeBatteryLevel(int i)
{
  batteryLevels[i] = batteryLevels[i - 1] + path[i - 1]->rechargingRate * chargingTimes[i - 1] -
                     vehicle.dischargingRate * inst->getTravelTime(path[i - 1], path[i]);
}

// Retorna o índice 'i' de desembarque (delivery) de um nó 'j' de embarque (pickup) da rota
int Route::getDeliveryIndexOf(int j)
{
  if (path[j]->type != Type::PICKUP)
    throw "O nó fornecido não é um ponto de embarque";

  for (int i = 1; i < path.size(); i++)
    if (path[i]->id == path[j]->id + inst->requestsAmount)
      return i;
}

// Retorna o índice 'i' de embarque (pickup) de um nó 'j' de desembarque (delivery) da rota
int Route::getPickupIndexOf(int j)
{
  if (path[j]->type != Type::DELIVERY)
    throw "O nó fornecido não é um ponto de desembarque";

  for (int i = 1; i < path.size(); i++)
    if (path[i]->id == path[j]->id - inst->requestsAmount)
      return i;
}
