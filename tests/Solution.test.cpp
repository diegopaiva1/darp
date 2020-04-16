#include "catch2/catch.hpp"

#include "algorithms/ReactiveGrasp.hpp"
#include "data-structures/Singleton.hpp"
#include "data-structures/Solution.hpp"
#include "utils/Timer.hpp"

namespace Test
{
  std::pair<Solution, double> solution;

  void buildSolution()
  {
    solution = ReactiveGrasp::solve(1000, 100, {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0});
  }
}

TEST_CASE("all requests should be served exactly once and in the SAME route")
{
  Test::buildSolution();

  bool allRequestsServedExactlyOnce = true;

  for (Request req : Singleton::getInstance()->requests) {
    int pickupCount   = 0;
    int deliveryCount = 0;

    for (Route route : Test::solution.first.routes) {
      for (Node *n : route.path) {
        if (n == req.pickup)
          pickupCount++;
        else if (n == req.delivery)
          deliveryCount++;
      }

      // Request had one of it's nodes served more than once in this route or there was a missing node
      if ((pickupCount != deliveryCount) && (pickupCount != 1 && deliveryCount != 1)) {
        allRequestsServedExactlyOnce = false;
        break;
      }
    }

    if (!allRequestsServedExactlyOnce)
      break;
  }

  REQUIRE(allRequestsServedExactlyOnce == true);
}

TEST_CASE("there should be at least one route")
{
  REQUIRE(Test::solution.first.routes.size() >= 1);
}

TEST_CASE("every route should begin and end in a depot")
{
  for (Route r : Test::solution.first.routes) {
    REQUIRE(r.path.front()->isDepot() == true);
    REQUIRE(r.path.back()->isDepot()  == true);
  }
}

TEST_CASE("there should be NO load violation")
{
  REQUIRE(Test::solution.first.loadViolation == 0);
}

TEST_CASE("there should be NO time window violation")
{
  REQUIRE(Test::solution.first.timeWindowViolation == 0.0);
}

TEST_CASE("there should be NO maximum ride time violation")
{
  REQUIRE(Test::solution.first.maxRideTimeViolation == 0.0);
}

TEST_CASE("there should be NO final battery level violation")
{
  REQUIRE(Test::solution.first.finalBatteryViolation == 0.0);
}

TEST_CASE("there should be NO order violation")
{
  REQUIRE(Test::solution.first.orderViolation == 0.0);
}

TEST_CASE("there should be NO battery level violation")
{
  REQUIRE(Test::solution.first.orderViolation == 0.0);
}

TEST_CASE("there should not be more routes in solution than existing vehicles")
{
  REQUIRE(Test::solution.first.routes.size() <= Singleton::getInstance()->vehicles.size());
}
