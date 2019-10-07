/**
 * @file    InsertionHeuristic.hpp
 * @author  Diego Paiva
 * @date    26/09/2019
 */

#ifndef INSERTIONHEURISTIC_H_INCLUDED
#define INSERTIONHEURISTIC_H_INCLUDED

#include "../data-structures/Instance.hpp"
#include "../data-structures/Solution.hpp"
#include "../utils/Prng.hpp"

class InsertionHeuristic
{
public:
  static Solution getSolution(Instance instance)
  {
    Solution solution;
    std::vector<Request*> requests;

    for (Request *r : instance.requests)
      requests.push_back(r);

    for (Vehicle *v : instance.vehicles)
      solution.routes.push_back(new Route(v));

    std::sort(requests.begin(), requests.end(), [](Request *&r1, Request *&r2) {
      return r1->getTimeWindowMedian() < r2->getTimeWindowMedian();
    });

    for (Route *route : solution.routes) {
      route->path.push_back(instance.getOriginDepot());
      route->path.push_back(instance.getDestinationDepot());
    }

    while (!requests.empty()) {
      Request *request = requests[0];

      performCheapestInsertion(request, solution);

      requests.erase(requests.begin());
    }

    return solution;
  }

  static void performCheapestInsertion(Request *&request, Solution &solution)
  {
    Route *best = new Route();
    best->path = solution.routes[0]->path;

    for (Route *r : solution.routes) {
      r->path.insert(r->path.begin() + 1, request->pickup);
      r->path.insert(r->path.begin() + 2, request->delivery);

      for (int i = 1; i < r->path.size() - 2; i++) {
        if (i != 1) {
          std::swap(r->path.at(i), r->path.at(i - 1));
          std::swap(r->path.at(i + 1), r->path.at(r->path.size() - 2));
        }

        if (r->getTotalDistance() < best->getTotalDistance())
          best->path = r->path;

        for (int j = i + 1; j < r->path.size() - 2; j++) {
          std::swap(r->path.at(j), r->path.at(j + 1));

          if (r->getTotalDistance() < best->getTotalDistance())
            best->path = r->path;
        }
      }
    }
  }
};

#endif // INSERTIONHEURISTIC_H_INCLUDED
