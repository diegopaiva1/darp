/**
 * @file    Grasp.hpp
 * @author  Diego Paiva
 * @date    26/09/2019
 */

#ifndef GRASP_H_INCLUDED
#define GRASP_H_INCLUDED

#define ITERATIONS 500

#include "../data-structures/Singleton.hpp"
#include "../data-structures/Solution.hpp"
#include "../utils/Prng.hpp"

// Initial parameters for the evaluation function
float alpha = 1.0;
float beta  = 1.0;
float gama  = 1.0;
float delta = Prng::generateFloatInRange(0.05, 0.10);

class Grasp
{
public:
  static Solution solve()
  {
    Solution best;
    std::vector<double> alphas = {0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50};

    for (int it = 1; it <= ITERATIONS; it++) {
      int index;
      int alphaIndex = Prng::generateIntegerInRange(0, alphas.size() - 1);
      Solution currSolution;
      std::vector<Request*> requests;

      for (Request *r : Singleton::getInstance()->requests)
        requests.push_back(r);

      for (Vehicle *v : Singleton::getInstance()->vehicles)
        currSolution.routes.push_back(new Route(v));

      std::sort(requests.begin(), requests.end(), [](Request *&r1, Request *&r2) {
        return r1->getTimeWindowMedian() < r2->getTimeWindowMedian();
      });

      for (Route *route : currSolution.routes) {
        // Na primeira iteração, selecionamos o melhor candidato da lista (guloso/míope)
        it == 1 ? index = 0
                : index = Prng::generateIntegerInRange(0, (int) (alphas[alphaIndex] * requests.size()));

        Request *request = requests[index];

        route->path.push_back(Singleton::getInstance()->getOriginDepot());
        route->path.push_back(Singleton::getInstance()->getDestinationDepot());
        route->path.insert(route->path.begin() + 1, request->pickup);
        route->path.insert(route->path.begin() + 2, request->delivery);

        requests.erase(requests.begin() + index);
      }

      while (!requests.empty()) {
        // Na primeira iteração, selecionamos o melhor candidato da lista (guloso/míope)
        it == 1 ? index = 0
                : index = Prng::generateIntegerInRange(0, (int) (alphas[alphaIndex] * requests.size()));
        Request *request = requests[index];
        performCheapestFeasibleInsertion(request, currSolution);
        requests.erase(requests.begin() + index);
      }

      float evaluation = 0.0;

      for (Route *r : currSolution.routes)
        evaluation += performEightStepEvaluationScheme(r, true);

      printf("c(s) = %f e f(s) = %f\n", currSolution.cost(), evaluation);

      // Priorizamos rotas com o menor número de veículos e menor custo
      if ((it == 1) || (currSolution.routes.size() < best.routes.size()) || (currSolution.routes.size() == best.routes.size() && evaluation == currSolution.cost() && evaluation < best.cost()))
        best = currSolution;
    }

    std::cout << best.cost() << '\n';

    return best;
  }

  static float performEightStepEvaluationScheme(Route *&r, bool flag)
  {
    int   loadViolation        = 0;
    float timeWindowViolation  = 0.0;
    float maxRideTimeViolation = 0.0;

    int size = r->path.size();
    r->arrivalTimes.clear();
    r->arrivalTimes.resize(size);
    r->serviceBeginningTimes.clear();
    r->serviceBeginningTimes.resize(size);
    r->departureTimes.clear();
    r->departureTimes.resize(size);
    r->waitingTimes.clear();
    r->waitingTimes.resize(size);
    r->ridingTimes.clear();
    r->ridingTimes.resize(size);
    r->load.clear();
    r->load.resize(size);
    r->batteryLevels.clear();
    r->batteryLevels.resize(size);


    // STEP 1: D_0 = e_0
    r->departureTimes[0] = r->path[0]->arrivalTime;

    // STEP 2: Compute A_i, W_i, B_i, D_i and Q_i for each vertex i in the route
    for (int i = 1; i < r->path.size(); i++) {
      computeLoad(r, i);

      if (r->load[i] > r->vehicle->capacity)
        goto step8;

      computeArrivalTime(r, i);
      computeServiceBeginningTime(r, i);

      if (r->serviceBeginningTimes[i] > r->path[i]->departureTime)
       goto step8;

      computeWaitingTime(r, i);
      computeDepartureTime(r, i);
    }

    float f0, waitingTimeSum;
    bool allRidingTimesRespected;

    // STEP 3: Compute F_0
    f0 = calculateForwardSlackTime(0, r);

    // STEP 4: Set D_0 = e_0 + min{f_0, sum(W_p)}
    waitingTimeSum = 0.0;

    for (int i = 1; i < r->path.size() - 1; i++)
      waitingTimeSum += r->waitingTimes[i];

    r->departureTimes[0] = r->path[0]->arrivalTime + std::min(f0, waitingTimeSum);

    // STEP 5: Update A_i, W_i, B_i and D_i for each vertex i in the route
    for (int i = 1; i < r->path.size(); i++) {
      computeArrivalTime(r, i);
      computeServiceBeginningTime(r, i);
      computeWaitingTime(r, i);
      computeDepartureTime(r, i);
    }

    // STEP 6: Compute L_i for each request in the route
    allRidingTimesRespected = true;

    for (int i = 1; i < r->path.size() - 1; i++) {
      if (r->path[i]->isPickup()) {
        computeRidingTime(r, i);

        if (r->ridingTimes[i] > r->path[i]->maxRideTime)
          allRidingTimesRespected = false;
      }
    }

    // if (allRidingTimesRespected)
    //   goto step8;

    // STEP 7: For every vertex i that is an origin
    for (int i = 1; i < r->path.size() - 1; i++) {
      if (r->path[i]->isPickup()) {
        // STEP 7.a - Compute F_i
        float forwardSlackTime = calculateForwardSlackTime(i, r);

        // STEP 7.b - Set W_i = W_i + min{F_i, sum(W_p)}; B_i = A_i + W_i; D_i = B_i + d_i
        float waitingTimeSum = 0.0;
        for (int p = i + 1; p < r->path.size() - 1; p++)
          waitingTimeSum += r->waitingTimes[p];

        r->waitingTimes[i] += std::min(forwardSlackTime, waitingTimeSum);
        r->serviceBeginningTimes[i] = r->arrivalTimes[i] + r->waitingTimes[i];
        r->departureTimes[i] = r->serviceBeginningTimes[i] + r->path[i]->serviceTime;

        // STEP 7.c - Update A_j, W_j, B_j and D_j for each vertex j that comes after i in the route
        for (int j = i + 1; j < r->path.size(); j++) {
          computeArrivalTime(r, j);
          computeServiceBeginningTime(r, j);
          computeWaitingTime(r, j);
          computeDepartureTime(r, j);
        }

        // STEP 7.d - Update L_j for each request j whose destination is after i
        bool rid = true;
        for (int j = i + 1; j < r->path.size() - 1; j++) {
          if (r->path[j]->isDelivery()) {
            int pickupIndex = getPickupIndexOf(r, j);
            computeRidingTime(r, pickupIndex);
          }

          if (r->ridingTimes[j] > r->path[j]->maxRideTime)
            rid = false;
        }

        // if (rid)
        //   goto step8;
      }
    }

  step8:
    // for (int i = 0; i < r->path.size(); i++) {
    //   computeLoad(r, i);
    //   computeArrivalTime(r, i);
    //   computeServiceBeginningTime(r, i);
    //   computeWaitingTime(r, i);
    //   computeDepartureTime(r, i);

    //   if (r->path[i]->isPickup())
    //     computeRidingTime(r, i);
    // }

    // STEP 8: Compute changes in violations of vehicle load, time window and ride time constraints
    for (int i = 1; i < r->path.size() - 1; i++) {
      if (r->path[i]->isPickup())
        maxRideTimeViolation += std::max(0.0f, r->ridingTimes[i] - r->path[i]->maxRideTime);

      loadViolation       += std::max(0, r->load[i] - r->vehicle->capacity);
      timeWindowViolation += std::max(0.0f, r->serviceBeginningTimes[i] - r->path[i]->departureTime);
    }

    if (flag) {
      float factor = 1 + delta;
      // Adjust the penalty parameters for each violation
      loadViolation        == 0 ? alpha /= factor : alpha *= factor;
      timeWindowViolation  == 0 ? beta  /= factor : beta  *= factor;
      maxRideTimeViolation == 0 ? gama  /= factor : gama  *= factor;

      // printf("%d - %f - %f\n", loadViolation, timeWindowViolation, maxRideTimeViolation);
      printf("%f, %f, %f\n", alpha, beta, gama);

      delta = Prng::generateFloatInRange(0.05, 0.10);
    }
    // return false;
    return r->getTotalDistance() + (alpha * loadViolation )+ (beta * timeWindowViolation) + (gama * maxRideTimeViolation);
  }

 static bool isFeasible(Solution s)
{
  for (Route *r : s.routes) {
    for (int i = 0; i < r->path.size(); i++) {
      if (r->serviceBeginningTimes[i] > r->path[i]->departureTime)
      {
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
    }
  }

  return true;
}

  /* O forward slack time é calculado como o menor de todos os slack times entre
   * o index (ponto da rota que se deseja calcular) e o ponto final da rota
   */
  static float calculateForwardSlackTime(int index, Route *r)
  {
    float forwardSlackTime;

    for (int j = index; j < r->path.size(); j++) {
      float waitingTimeSum = 0.0;

      for (int p = index + 1; p <= j; p++)
        waitingTimeSum += r->waitingTimes[p];

      float userRideTimeWithDeliveryAtJ = 0.0;
      bool jMinusNIsVisitedBeforeIndex  = false;

      for (int p = 0; p <= index; p++)
        if (r->path[p]->id == j - Singleton::getInstance()->requestsAmount < index)
          jMinusNIsVisitedBeforeIndex = true;

      if (r->path[j]->type == Type::DELIVERY && jMinusNIsVisitedBeforeIndex)
        userRideTimeWithDeliveryAtJ = r->ridingTimes[getPickupIndexOf(r, j)];

      float slackTime;

      if (r->path[index]->type == Type::PICKUP)
        slackTime = waitingTimeSum +
                    std::max(0.0f, std::min(r->path[j]->departureTime - r->serviceBeginningTimes[j],
                                            r->path[index]->maxRideTime - userRideTimeWithDeliveryAtJ));
      else if (index == 0)
        slackTime = waitingTimeSum + std::max(0.0f, r->path[j]->departureTime - r->serviceBeginningTimes[j]);
      else
        slackTime = 0.0;

      if (j == index || slackTime < forwardSlackTime)
        forwardSlackTime = slackTime;
    }

    return forwardSlackTime;
  }


  static void performCheapestFeasibleInsertion(Request *&request, Solution &solution)
  {
    float bestCost = std::numeric_limits<float>::max();
    int routeId = -9999;
    int bestPickupIndex = -9999;
    int bestDeliveryIndex = -9999;

    for (int i = 0; i < solution.routes.size(); i++) {
      Route *r = solution.routes[i];

      for (int p = 1; p < r->path.size(); p++) {
        r->path.insert(r->path.begin() + p, request->pickup);

        for (int d = p + 1; d < r->path.size(); d++) {
          r->path.insert(r->path.begin() + d, request->delivery);

          float f = performEightStepEvaluationScheme(r, false);

          if (f == r->getTotalDistance() && f < bestCost) {
            bestCost = f;
            routeId = i;
            bestPickupIndex = p;
            bestDeliveryIndex = d;
          }

          r->path.erase(r->path.begin() + d);
        }

        r->path.erase(r->path.begin() + p);
      }
    }

    if (routeId == -9999 && bestPickupIndex == -9999 && bestDeliveryIndex == -9999) {
      Route *route = new Route(new Vehicle(solution.routes.size() + 1));
      route->path.push_back(Singleton::getInstance()->getOriginDepot());
      route->path.push_back(request->pickup);
      route->path.push_back(request->delivery);
      route->path.push_back(Singleton::getInstance()->getDestinationDepot());
      solution.routes.push_back(route);
    }
    else {
      Route *best = solution.routes.at(routeId);
      best->path.insert(best->path.begin() + bestPickupIndex,   request->pickup);
      best->path.insert(best->path.begin() + bestDeliveryIndex, request->delivery);
    }
  }

  static void computeLoad(Route *&r, int i)
  {
    if (i == 0)
      r->load[i] = 0;
    else
      r->load[i] = r->load[i - 1] + r->path[i]->load;
  }

  static void computeArrivalTime(Route *&r, int i)
  {
    if (i == 0)
      r->arrivalTimes[i] = 0;
    else
      r->arrivalTimes[i] = r->departureTimes[i - 1] + Singleton::getInstance()->getTravelTime(r->path[i - 1], r->path[i]);
  }

  static void computeServiceBeginningTime(Route *&r, int i)
  {
    if (i == 0)
      r->serviceBeginningTimes[i] = r->departureTimes[i];
    else
      r->serviceBeginningTimes[i] = std::max(r->arrivalTimes[i], r->path[i]->arrivalTime);
  }

  static void computeWaitingTime(Route *&r, int i)
  {
    if (i == 0)
      r->waitingTimes[i] = 0;
    else
      r->waitingTimes[i] = r->serviceBeginningTimes[i] - r->arrivalTimes[i];
  }

  static void computeDepartureTime(Route *&r, int i)
  {
    if (i == r->path.size() - 1)
      r->departureTimes[i] = 0;
    else
      r->departureTimes[i] = r->serviceBeginningTimes[i] + r->path[i]->serviceTime;
  }

  static void computeRidingTime(Route *&r, int i)
  {
    r->ridingTimes[i] = r->serviceBeginningTimes[getDeliveryIndexOf(r, i)] - r->departureTimes[i];
  }

  // Retorna o índice 'i' de desembarque (delivery) de um nó 'j' de embarque (pickup) da rota
  static int getDeliveryIndexOf(Route *&r, int j)
  {
    if (r->path[j]->type != Type::PICKUP)
      throw "O nó fornecido não é um ponto de embarque";

    for (int i = 1; i < r->path.size(); i++)
      if (r->path[i]->id == r->path[j]->id + Singleton::getInstance()->requestsAmount)
        return i;
  }

  // Retorna o índice 'i' de embarque (pickup) de um nó 'j' de desembarque (delivery) da rota
  static int getPickupIndexOf(Route *&r, int j)
  {
    if (r->path[j]->type != Type::DELIVERY)
      throw "O nó fornecido não é um ponto de desembarque";

    for (int i = 1; i < r->path.size(); i++)
      if (r->path[i]->id == r->path[j]->id - Singleton::getInstance()->requestsAmount)
        return i;
  }
};

#endif // GRASP_H_INCLUDED
