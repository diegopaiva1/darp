/**
 * @file   ReactiveGrasp.cpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "data-structures/Singleton.hpp"
#include "algorithms/ReactiveGrasp.hpp"
#include "utils/Timer.hpp"
#include "utils/Display.hpp"

std::tuple<Solution, double, uint, int>
ReactiveGrasp::solve(int iterations = 100, int blocks = 10, std::vector<double> alphas = {1.0})
{
  // Starting a clock to count algorithm's run time
  Timer timer;

  // Use std::random_device to generate seed to Random engine
  uint seed = std::random_device{}();
  Random::seed(1231595941);

  std::vector<RandomParam> randomParams (alphas.size());

  // Initializing each alpha as random parameter
  for (int i = 0; i < alphas.size(); i++) {
    RandomParam rp;

    rp.alpha          = alphas[i];
    rp.probability    = 1.0/alphas.size();
    rp.q              = 0.0;
    rp.cumulativeCost = 0.0;
    rp.count          = 0;

    randomParams[i] = rp;
  }

  Solution best;
  int optimalIteration = 0;

  for (int it = 1; it <= iterations; it++) {
    int index;
    double alpha;

    // Go full greedy at first iteration
    if (it == 1) {
      alpha = 0.0;
    }
    else {
      index = chooseRandomParamIndex(randomParams);
      alpha = randomParams[index].alpha;
      randomParams[index].count++;
    }

    Solution curr = buildGreedyRandomizedSolution(alpha);
    curr = rvnd(curr);

    if (it == 1 or curr.routes.size() < best.routes.size() or
       (curr.routes.size() == best.routes.size() && curr.cost < best.cost)) {
      best = curr;

      if (optimalIteration == 0 && fabs(best.cost - inst->optimalSolutions[inst->name]) < 0.01)
        optimalIteration = it;
    }

    if (it % blocks == 0) {
      for (int i = 0; i < randomParams.size(); i++) {
        double avg = randomParams[i].count > 0 ? randomParams[i].cumulativeCost/randomParams[i].count : 0;

        if (avg > 0)
          randomParams[i].q = best.cost/avg;
      }

      updateProbabilities(randomParams);
    }

    // Erase any route without requests
    for (auto k = curr.routes.begin(); k != curr.routes.end(); )
      k = (k->path.size() <= 2) ? curr.routes.erase(k) : k + 1;

    if (it != 1) {
      int param = curr.routes.size() > inst->vehicles.size() ? 1000 : 0;
      randomParams[index].cumulativeCost += curr.cost + param * curr.routes.size();
    }

    // printf("\n");
    // for (int i = 0; i < randomParams.size(); i++) {
    //   double avg = randomParams[i].count > 0 ? randomParams[i].cumulativeCost/randomParams[i].count : 0;
    //   printf("Avg %.2f = %.2f (%.2f)\n", randomParams[i].alpha, avg, randomParams[i].probability);
    // }

    Display::printProgress(best, (double) it/iterations);
  }

  double elapsedTime = timer.elapsedInMinutes();

  Display::printSolutionInfoWithElapsedTime(best, elapsedTime);

  for (int i = 0; i < randomParams.size(); i++) {
    RandomParam rp = randomParams[i];
    printf("%2d. %.2f - %d times (%.2f%%)\n", i + 1, rp.alpha, rp.count, rp.probability);
  }

  printf("%d\n", optimalIteration);

  return std::make_tuple(best, elapsedTime, seed, optimalIteration);
}

Solution ReactiveGrasp::buildGreedyRandomizedSolution(double alpha)
{
  Solution solution;

  for (Vehicle &v : inst->vehicles)
    solution.routes.push_back(Route(v));

  for (Route &route : solution.routes) {
    route.path.push_back(inst->getOriginDepot());
    route.path.push_back(inst->getDestinationDepot());
  }

  struct Candidate
  {
    Route   route;
    Request request;
  };

  std::vector<Candidate> candidates;

  // Initialize candidates
  for (Request req : inst->requests)
    candidates.push_back({getBestInsertion(req, solution), req});

  while (!candidates.empty()) {
    std::sort(candidates.begin(), candidates.end(), [] (Candidate &c1, Candidate &c2) {
      return c1.route.cost < c2.route.cost;
    });

    int index = Random::get(0, (int) (alpha * (candidates.size() - 1)));
    Candidate chosen = candidates[index];

    // Request could not be feasibly inserted, so we create a new route (thus solution will be infeasible)
    if (chosen.route.cost == MAXFLOAT) {
      Route newest = createRoute(solution);
      newest.path.push_back(inst->getOriginDepot());
      newest.path.push_back(chosen.request.pickup);
      newest.path.push_back(chosen.request.delivery);
      newest.path.push_back(inst->getDestinationDepot());
      performEightStepEvaluationScheme(newest);
      solution.routes.push_back(newest);
    }
    else {
      solution.routes.at(chosen.route.vehicle.id - 1) = chosen.route;
    }

    candidates.erase(candidates.begin() + index);

    // Update candidates
    for (int i = 0; i < candidates.size(); i++)
      if (candidates[i].route == chosen.route)
        candidates[i] = {getBestInsertion(candidates[i].request, solution), candidates[i].request};
  }

  solution.updateCost();

  return solution;
}

Solution ReactiveGrasp::rvnd(Solution s)
{
  std::vector<int> neighborhoods = {1, 2, 3};

  while (!neighborhoods.empty()) {
    Solution neighbor;
    int k = Random::get(0, (int) neighborhoods.size() - 1);

    switch (neighborhoods.at(k)) {
      case 1:
        neighbor = reinsert(s);
        break;
      case 2:
        neighbor = swapZeroOne(s);
        break;
      case 3:
        neighbor = swapOneOne(s);
        break;
    }

    if (neighbor.cost < s.cost) {
      s = neighbor;
      neighborhoods = {1, 2, 3};
    }
    else {
      neighborhoods.erase(neighborhoods.begin() + k);
    }
  }

  return s;
}

Solution ReactiveGrasp::swapZeroOne(Solution s)
{
  for (int k1 = 0; k1 < s.routes.size(); k1++) {
    for (int i = 1; i < s.routes[k1].path.size() - 1; i++) {
      Node *node = s.routes[k1].path[i];

      if (node->isPickup()) {
        Request req = inst->getRequest(node);
        Route   r1  = s.routes[k1];

        r1.path.erase(std::remove(r1.path.begin(), r1.path.end(), req.pickup),   r1.path.end());
        r1.path.erase(std::remove(r1.path.begin(), r1.path.end(), req.delivery), r1.path.end());

        performEightStepEvaluationScheme(r1);

        for (int k2 = 0; k2 < s.routes.size(); k2++) {
          if (k1 != k2) {
            Route r2 = performCheapestFeasibleInsertion(req, s.routes[k2]);

            if (r1.cost + r2.cost < s.routes[k1].cost + s.routes[k2].cost) {
              s.routes[k1] = r1;
              s.routes[k2] = r2;
              k1 = 0;
              i  = 1;
              break;
            }
          }
        }
      }
    }
  }

  s.updateCost();

  return s;
}

Solution ReactiveGrasp::swapOneOne(Solution s)
{
  std::vector<int> possibleRoutes;

  // Only routes with at least one request are eligible
  for (int k = 0; k < s.routes.size(); k++)
    if (s.routes[k].path.size() > 2)
      possibleRoutes.push_back(k);

  // To perform the movement, there must be at least two routes with requests to be swapped
  if (possibleRoutes.size() >= 2) {
    bool improved;
    bool eligible;

    do {
      improved = false;

      // Select two distinct random routes
      int k1 = *Random::get(possibleRoutes);
      int k2 = *Random::get(possibleRoutes);

      while (k2 == k1)
        k2 = *Random::get(possibleRoutes);

      Route r1 = s.routes[k1];
      Route r2 = s.routes[k2];

      // Select two random requests in each route
      Node *node1 = *Random::get(r1.path.begin() + 1, r1.path.end() - 1);
      Node *node2 = *Random::get(r2.path.begin() + 1, r2.path.end() - 1);

      Request req1 = inst->getRequest(node1);
      Request req2 = inst->getRequest(node2);

      // Remove req1 from r1 and req2 from r2
      r1.path.erase(std::remove(r1.path.begin(), r1.path.end(), req1.pickup),   r1.path.end());
      r1.path.erase(std::remove(r1.path.begin(), r1.path.end(), req1.delivery), r1.path.end());

      r2.path.erase(std::remove(r2.path.begin(), r2.path.end(), req2.pickup),   r2.path.end());
      r2.path.erase(std::remove(r2.path.begin(), r2.path.end(), req2.delivery), r2.path.end());

      // Insert req2 in r1 and req1 in r2
      r1 = performCheapestFeasibleInsertion(req2, r1);
      r2 = performCheapestFeasibleInsertion(req1, r2);

      if (r1.cost + r2.cost < s.routes[k1].cost + s.routes[k2].cost) {
        s.routes[k1] = r1;
        s.routes[k2] = r2;
        improved = true;
      }
    }
    while (improved);
  }

  s.updateCost();

  return s;
}

Solution ReactiveGrasp::reinsert(Solution s)
{
  for (int k = 0; k < s.routes.size(); k++) {
    for (int i = 1; i < s.routes[k].path.size() - 1; i++) {
      Node *node = s.routes[k].path[i];

      if (node->isPickup()) {
        // Remove request from the route
        Request req = inst->getRequest(node);
        Route   r   = s.routes[k];

        r.path.erase(std::remove(r.path.begin(), r.path.end(), req.pickup),   r.path.end());
        r.path.erase(std::remove(r.path.begin(), r.path.end(), req.delivery), r.path.end());

        // Reinsert the request
        r = performCheapestFeasibleInsertion(req, r);

        if (r.cost < s.routes[k].cost) {
          s.routes[k] = r;
          i = 1;
        }
      }
    }
  }

  s.updateCost();

  return s;
}

Route ReactiveGrasp::createRoute(Solution &s)
{
  Vehicle newest(s.routes.size() + 1);
  Vehicle v = s.routes.at(0).vehicle;

  newest.batteryCapacity           = v.batteryCapacity;
  newest.capacity                  = v.capacity;
  newest.dischargingRate           = v.dischargingRate;
  newest.initialBatteryLevel       = v.initialBatteryLevel;
  newest.minFinalBatteryRatioLevel = v.minFinalBatteryRatioLevel;

  return Route(newest);
}

Route ReactiveGrasp::getBestInsertion(Request request, Solution solution)
{
  Route best;

  for (int k = 0; k < solution.routes.size(); k++) {
    Route r = performCheapestFeasibleInsertion(request, solution.routes[k]);

    if (k == 0 || r.cost < best.cost)
      best = r;
  }

  return best;
}

Route ReactiveGrasp::performCheapestFeasibleInsertion(Request req, Route r)
{
  // Best insertion starts with infinity cost, we will update it during the search
  Route best = r;
  best.cost = MAXFLOAT;

  for (int p = 1; p < r.path.size(); p++) {
    r.path.insert(r.path.begin() + p, req.pickup);

    for (int d = p + 1; d < r.path.size(); d++) {
      r.path.insert(r.path.begin() + d, req.delivery);

      if (performEightStepEvaluationScheme(r) && r.cost < best.cost)
        best = r;

      r.path.erase(r.path.begin() + d);
    }

    r.path.erase(r.path.begin() + p);
  }

  return best;
}

bool ReactiveGrasp::performEightStepEvaluationScheme(Route &r)
{
  int size = r.path.size();

  r.arrivalTimes.clear();
  r.arrivalTimes.resize(size);
  r.serviceBeginningTimes.clear();
  r.serviceBeginningTimes.resize(size);
  r.departureTimes.clear();
  r.departureTimes.resize(size);
  r.waitingTimes.clear();
  r.waitingTimes.resize(size);
  r.rideTimes.clear();
  r.rideTimes.resize(size);
  r.load.clear();
  r.load.resize(size);
  r.batteryLevels.clear();
  r.batteryLevels.resize(size);
  r.chargingTimes.clear();
  r.chargingTimes.resize(size);
  r.rideTimeExcesses.clear();
  r.rideTimeExcesses.resize(size);

  // Compute every node index in the route before evaluation
  for (int i = 1; i < r.path.size() - 1; i++)
    r.path[i]->index = i;

  double forwardTimeSlackAtBeginning;

  STEP1:
    r.departureTimes[0]        = r.path[0]->arrivalTime;
    r.serviceBeginningTimes[0] = r.departureTimes[0];
    r.batteryLevels[0]         = r.vehicle.initialBatteryLevel;

  STEP2:
    for (int i = 1; i < r.path.size(); i++) {
      r.computeLoad(i);

      // Violated vehicle capacity, that's an irreparable violation
      if (r.load[i] > r.vehicle.capacity)
        goto STEP8;

      r.computeArrivalTime(i);
      r.computeServiceBeginningTime(i);

      // Violated time windows, that's an irreparable violation
      if (r.serviceBeginningTimes[i] > r.path[i]->departureTime)
        goto STEP8;

      r.computeWaitingTime(i);
      r.computeChargingTime(i);
      r.computeBatteryLevel(i);

      // Violated battery level, that's an irreparable violation
      if (r.batteryLevels[i] < 0.0 /* || batteryLevels[i] > vehicle.batteryCapacity */)
        goto STEP8;

      r.computeDepartureTime(i);
    }

  STEP3:
    forwardTimeSlackAtBeginning = r.computeForwardTimeSlack(0);

  STEP4:
    r.departureTimes[0] = r.path[0]->arrivalTime + std::min(
      forwardTimeSlackAtBeginning, std::accumulate(r.waitingTimes.begin() + 1, r.waitingTimes.end() - 1, 0.0)
    );

    r.serviceBeginningTimes[0] = r.departureTimes[0];

  STEP5:
    for (int i = 1; i < r.path.size(); i++) {
      r.computeArrivalTime(i);
      r.computeServiceBeginningTime(i);
      r.computeWaitingTime(i);
      r.computeChargingTime(i);
      r.computeDepartureTime(i);
    }

  STEP6:
    for (int i = 1; i < r.path.size() - 1; i++)
      if (r.path[i]->isPickup())
        r.computeRideTime(i);

  STEP7:
    for (int j = 1; j < r.path.size() - 1; j++) {
      if (r.path[j]->isPickup() && r.load[j] == 1) {
        STEP7a:
          double forwardTimeSlack = r.computeForwardTimeSlack(j);

        STEP7b:
          r.waitingTimes[j] += std::min(
            forwardTimeSlack, std::accumulate(r.waitingTimes.begin() + j + 1, r.waitingTimes.end() - 1, 0.0)
          );

          r.serviceBeginningTimes[j] = r.arrivalTimes[j] + r.waitingTimes[j];
          r.departureTimes[j] = r.serviceBeginningTimes[j] + r.path[j]->serviceTime;

        STEP7c:
          for (int i = j + 1; i < r.path.size(); i++) {
            r.computeArrivalTime(i);
            r.computeServiceBeginningTime(i);
            r.computeWaitingTime(i);
            r.computeChargingTime(i);
            r.computeDepartureTime(i);
          }

        STEP7d:
          for (int i = j + 1; i < r.path.size() - 1; i++)
            if (r.path[i]->isDelivery())
              r.computeRideTime(inst->getRequest(r.path[i]).pickup->index);
      }
    }

  for (int i = 0; i < r.path.size(); i++)
    if (r.path[i]->isPickup())
      r.computeRideTimeExcess(i);

  STEP8:
    r.travelTime     = 0.0;
    r.excessRideTime = 0.0;

    bool   batteryLevelViolation  = false;
    int    loadViolation          = 0;
    double timeWindowViolation    = 0.0;
    double maxRideTimeViolation   = 0.0;
    double finalBatteryViolation  = 0.0;

    for (int i = 0; i < r.path.size(); i++) {
      if (i < r.path.size() - 1)
        r.travelTime += inst->getTravelTime(r.path[i], r.path[i + 1]);

      r.excessRideTime += r.rideTimeExcesses[i];

      if (r.path[i]->isPickup())
        maxRideTimeViolation += std::max(0.0, r.rideTimes[i] - r.path[i]->maxRideTime);

      loadViolation += std::max(0, r.load[i] - r.vehicle.capacity);
      timeWindowViolation += std::max(0.0, r.serviceBeginningTimes[i] - r.path[i]->departureTime);

      if (r.batteryLevels[i] < 0 || r.batteryLevels[i] > r.vehicle.batteryCapacity)
        batteryLevelViolation = true;
    }

    finalBatteryViolation += std::max(
      0.0, r.vehicle.batteryCapacity * r.vehicle.minFinalBatteryRatioLevel - r.batteryLevels[r.path.size() - 1]
    );

    r.cost = 0.75 * r.travelTime + 0.25 * r.excessRideTime;

    double violationSum = loadViolation + timeWindowViolation + maxRideTimeViolation +
                          finalBatteryViolation + batteryLevelViolation;

    return std::fpclassify(violationSum) == FP_ZERO;
}

int ReactiveGrasp::chooseRandomParamIndex(std::vector<RandomParam> randomParams)
{
  double rand = Random::get(0.0, 1.0);
  double sum  = 0.0;

  for (int i = 0; i < randomParams.size(); i++) {
    sum += randomParams[i].probability;

    if (rand <= sum)
      return i;
  }

  return 0;
}

void ReactiveGrasp::updateProbabilities(std::vector<RandomParam> &randomParams)
{
  double qsum = 0.0;

  for (RandomParam rp : randomParams)
    qsum += rp.q;

  for (int i = 0; i < randomParams.size(); i++)
    randomParams[i].probability = randomParams[i].q/qsum;
}
