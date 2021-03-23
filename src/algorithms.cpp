/**
 * @file   algorithms.cpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "algorithms.hpp"
#include "instance.hpp"
#include "gnuplot.hpp"

#include <cfloat> // FLT_MAX
#include <omp.h>  // OpenMP

namespace algorithms
{
  using namespace details;

  Run grasp(int iterations, double random_param, int thread_count)
  {
    if (thread_count < 1 || thread_count > omp_get_max_threads())
      thread_count = omp_get_max_threads();

    Run run;
    run.best.cost = FLT_MAX;

    double start = omp_get_wtime();

    #pragma omp parallel num_threads(thread_count)
    {
      // Use std::random_device to generate seed to Random engine to each thread
      unsigned int seed = std::random_device{}();
      Random::seed(seed);

      #pragma omp critical
      run.seeds.push_back(seed);

      #pragma omp for
      for (int it = 1; it <= iterations; it++) {
        Solution init = construct_greedy_randomized_solution(random_param);

        if (!init.feasible())
          init = repair(init);

        Solution curr = vnd(init);

        #pragma omp critical
        if (curr.feasible() && curr.cost < run.best.cost) {
          run.best = curr;
          run.init = init;
        }
      }
    }

    run.best.delete_empty_routes();
    run.init.delete_empty_routes();

    double finish = omp_get_wtime();
    run.elapsed_seconds = finish - start;

    return run;
  }

  Run ils(int max_iterations, int no_improvement_iterations, double random_param)
  {
    Run run;
    double start = omp_get_wtime();

    // Use std::random_device to generate seed to Random engine to each thread
    unsigned int seed = std::random_device{}();
    Random::seed(seed);
    run.seeds.push_back(seed);

    do {
      run.init = construct_greedy_randomized_solution(random_param);

      if (!run.init.feasible())
        run.init = repair(run.init);
    }
    while (!run.init.feasible());

    run.best = vnd(run.init);

    for (int it = 0, n = 0; it <= max_iterations; it++, n++) {
      Solution s = vnd(perturb(run.best));

      if (s.feasible() && s.cost < run.best.cost) {
        run.best = s;
        n = 0;
      }
      else {
        n++;
      }

      if (n == no_improvement_iterations)
        break;
    }

    run.init.delete_empty_routes();
    run.best.delete_empty_routes();

    double finish = omp_get_wtime();
    run.elapsed_seconds = finish - start;

    return run;
  }

  namespace details
  {
    Solution construct_greedy_randomized_solution(double random_param)
    {
      Solution solution;

      for (Vehicle *v : inst.vehicles) {
        Route r = Route(v);
        r.path.push_back(inst.get_depot());
        r.path.push_back(inst.get_depot());
        solution.add_route(r);
      }

      struct Candidate {
        Route route;
        Request *request;
      };

      std::vector<Candidate> candidates;

      // Init candidates
      for (Request *req : inst.requests)
        candidates.push_back({get_cheapest_insertion(req, solution), req});

      while (!candidates.empty()) {
        std::sort(candidates.begin(), candidates.end(), [] (const Candidate &c1, const Candidate &c2) {
          return c1.route.cost < c2.route.cost;
        });

        auto chosen_candidate = Random::get(candidates.begin(), candidates.begin() + (int) (random_param * candidates.size()));

        if (chosen_candidate->route.feasible()) {
          solution.add_route(chosen_candidate->route);
        }
        else {
          // Activate new vehicle to accomodate the request (thus solution will be infeasible)
          Vehicle *v = new Vehicle(solution.routes.size() + 1, inst.vehicles[0]->capacity, inst.vehicles[0]->max_route_duration);
          Route r(v);

          r.path.push_back(inst.get_depot());
          r.path.push_back(chosen_candidate->request->pickup);
          r.path.push_back(chosen_candidate->request->delivery);
          r.path.push_back(inst.get_depot());
          r.evaluate();
          solution.add_route(r);
        }

        candidates.erase(chosen_candidate);

        // Update candidates
        for (Candidate &c : candidates)
          c.route = get_cheapest_insertion(c.request, solution);
      }

      return solution;
    }

    Route get_cheapest_insertion(Request *req, Solution s)
    {
      Route best;
      double delta = best.cost = FLT_MAX;

      for (std::pair<Vehicle*, Route> pair : s.routes) {
	      Route r = pair.second;
        Route curr = get_cheapest_insertion(req, r);

        if (curr.feasible() && curr.cost - r.cost < delta) {
          delta = curr.cost - r.cost;
          best = curr;
        }
      }

      return best;
    }

    Route get_cheapest_insertion(Request *req, Route r)
    {
      Route best;
      best.cost = FLT_MAX;

      for (int p = 1; p < r.path.size(); p++) {
        bool discard = false;
        r.insert_node(req->pickup, p);

        if ((r.get_earliest_time(p) <= r.path[p]->departure_time) && (r.get_load(p - 1) + r.path[p]->load <= r.vehicle->capacity)) {
          for (int d = p + 1; d < r.path.size(); d++) {
            r.insert_node(req->delivery, d);

            if (r.cost < best.cost) {
              for (int i = p + 1; i <= d; i++) {
                if ((r.get_earliest_time(i) > r.path[i]->departure_time) || (r.get_load(i - 1) + r.path[i]->load > r.vehicle->capacity)) {
                  discard = true;
                  goto ERASE_DELIVERY;
                }
              }

              double time_gap_between_pickup_delivery = r.get_earliest_time(d) - r.path[p]->departure_time - r.path[p]->service_time;

              if (time_gap_between_pickup_delivery > r.path[p]->max_ride_time) {
                discard = true;
                goto ERASE_DELIVERY;
              }
              else if (r.evaluate()) {
                best = r;
              }
            }

            ERASE_DELIVERY:
            r.erase_node(d);

            if (discard)
              goto ERASE_PICKUP;
          }
        }

        ERASE_PICKUP:
        r.erase_node(p);
      }

      return best;
    }

    Solution vnd(Solution s, bool use_randomness)
    {
      // Only feasible solutions are allowed
      if (!s.feasible())
        return s;

      std::vector<Move> moves = {two_opt_star, reinsert, shift_1_0};

      for (int k = 0; k < moves.size(); /* conditional update */) {
        auto move = use_randomness ? Random::get(moves.begin() + k, moves.end()) : moves.begin() + k;
        Solution neighbor = (*move)(s);

        if (neighbor.cost < s.cost) {
          s = neighbor;
          k = 0;
        }
        else {
          k++;
        }
      }

      return s;
    }

    Solution reinsert(Solution s)
    {
      #ifdef DEBUG
        printf("\n\033[1m\033[33m-> Entering reinsert operator...\033[0m\n");
      #endif

      for (std::pair<Vehicle*, Route> pair : s.routes) {
	      Route r = pair.second;
        Route best_reinsertion = r;

        #ifdef DEBUG
          printf("\n<<<<<< Initial cost of route %d = %.2f >>>>>>\n", best_reinsertion.vehicle->id, best_reinsertion.cost);
        #endif

        // Perform reinsert only in routes with more than one request accommodated
        if (r.path.size() > 4) {
          for (int i = 0; i < r.path.size(); i++) {
            Node *node = r.path[i];

            if (node->is_pickup()) {
              Request *req = inst.get_request(node);
              Route curr = r;

              curr.erase_request(req);
              curr = get_cheapest_insertion(req, curr);

              if (curr.cost < best_reinsertion.cost)
                best_reinsertion = curr;

              #ifdef DEBUG
                printf("\nReinserting request (%d, %d) in route %d\n", req->pickup->id, req->delivery->id, r.vehicle->id);
                printf("\t-> ");

                for (Node *n : r.path)
                  if (n == req->pickup || n == req->delivery)
                    printf("\033[1m\033[31m%d\033[0m ", n->id);
                  else
                    printf("%d ", n->id);

                printf("(c = %.2lf)\n", r.cost);
                printf("\t-> ");

                for (Node *n : curr.path)
                  if (n == req->pickup || n == req->delivery)
                    printf("\033[1m\033[32m%d\033[0m ", n->id);
                  else
                    printf("%d ", n->id);

                if (curr.cost < r.cost)
                  printf("(c = \033[1m\033[32m%.2lf\033[0m)\n", curr.cost);
                else
                  printf("(c = %.2lf)\n", curr.cost);
              #endif
            }
          }
        }

        #ifdef DEBUG
          printf("\n<<<<<< Updated cost of route %d = %.2f >>>>>>\n", best_reinsertion.vehicle->id, best_reinsertion.cost);
        #endif

        s.add_route(best_reinsertion);
      }

      return s;
    }

    Solution repair(Solution s)
    {
      int extra_vehicles = s.routes.size() - inst.vehicles.size();

      #ifdef DEBUG
        printf("\n\033[1m\033[33m-> Repairing infeasible solution with %d extra vehicle(s)...\033[0m\n", extra_vehicles);
      #endif

      // Need a vector to sort routes by decreasing cost
      std::vector<std::pair<Vehicle*, Route>> routes(s.routes.begin(), s.routes.end());

      std::sort(routes.begin(), routes.end(), [] (const std::pair<Vehicle*, Route> &p1, const std::pair<Vehicle*, Route> &p2) {
        return p1.second.cost > p2.second.cost;
      });

      #ifdef DEBUG
        int count = 0;

        printf("\nRoutes ordered by decreasing cost:\n");
        for (auto [v, r] : routes) {
          const char* color = count < extra_vehicles ? "\033[1m\033[33m" : "\033[0m";
          printf("\t%sR%d: ", color, v->id);

          for (Node *n : r.path)
            printf("%d ", n->id);

          printf("%s(c = %.2lf)\033[0m\n", color, r.cost);
          count++;
        }
      #endif

      std::vector<Request*> unplanned;

      for (int i = 0; i < extra_vehicles; i++) {
        for (Node *node : routes[i].second.path)
          if (node->is_pickup())
            unplanned.push_back(inst.get_request(node));

        s.routes.erase(routes[i].first);
      }

      while (!unplanned.empty()) {
        auto request = Random::get(unplanned);
        Route best = get_cheapest_insertion(*request, s);

        if (!best.feasible()) {
          #ifdef DEBUG
            printf("\n\t-> No feasible insertion found! Solution will remain infeasible.\n");
          #endif

          // Add a null vehicle to make solution infeasible again
          s.add_route(Route(nullptr));
          return s;
        }

        s.add_route(best);
        unplanned.erase(request);
      }

      #ifdef DEBUG
        printf("\n\t\033[1m\033[32mSolution repaired successfully!\033[0m\n");

        for (auto pair : s.routes) {
          Vehicle *v = pair.first;
          Route r = pair.second;

          printf("\tR%d: ", v->id);

          for (Node *node : r.path)
            printf("%d ", node->id);

          printf("(c = %.2f)\n", r.cost);
        }
      #endif

      return s;
    }

    Solution perturb(Solution s)
    {
      #ifdef DEBUG
        printf("\n\033[1m\033[33m-> Perturbing solution...\033[0m\n\n");

        for (auto pair : s.routes) {
          Vehicle *v = pair.first;
          Route r = pair.second;

          printf("R%d: ", v->id);

          for (Node *node : r.path)
            printf("%d ", node->id);

          printf("(c = %.2f)\n", r.cost);
        }
      #endif

      int non_empty_routes = 0;

      for (auto pair : s.routes)
        if (!pair.second.empty())
          non_empty_routes++;

      if (non_empty_routes < 3)
        return s;

      Vehicle *v1, *v2, *v3;

      do {
        v1 = Random::get(s.routes)->first;
      }
      while (s.routes[v1].empty());

      do {
        v2 = Random::get(s.routes)->first;
      }
      while (s.routes[v2].empty() || v2 == v1);

      do {
        v3 = Random::get(s.routes)->first;
      }
      while (s.routes[v3].empty() || v3 == v1 || v3 == v2);

      Request *req1 = inst.get_request(s.routes[v1].path[Random::get(1, (int) s.routes[v1].path.size() - 2)]);
      Request *req2 = inst.get_request(s.routes[v2].path[Random::get(1, (int) s.routes[v2].path.size() - 2)]);
      Request *req3 = inst.get_request(s.routes[v3].path[Random::get(1, (int) s.routes[v3].path.size() - 2)]);

      // s.routes[v1].erase_request(req1);
      // s.routes[v2].erase_request(req2);
      // s.routes[v3].erase_request(req3);

      s.routes[v1].path.erase(std::remove(s.routes[v1].path.begin(), s.routes[v1].path.end(), req1->pickup), s.routes[v1].path.end());
      s.routes[v1].path.erase(std::remove(s.routes[v1].path.begin(), s.routes[v1].path.end(), req1->delivery), s.routes[v1].path.end());

      s.routes[v2].path.erase(std::remove(s.routes[v2].path.begin(), s.routes[v2].path.end(), req2->pickup), s.routes[v2].path.end());
      s.routes[v2].path.erase(std::remove(s.routes[v2].path.begin(), s.routes[v2].path.end(), req2->delivery), s.routes[v2].path.end());

      s.routes[v3].path.erase(std::remove(s.routes[v3].path.begin(), s.routes[v3].path.end(), req3->pickup), s.routes[v3].path.end());
      s.routes[v3].path.erase(std::remove(s.routes[v3].path.begin(), s.routes[v3].path.end(), req3->delivery), s.routes[v3].path.end());

      #ifdef DEBUG
        printf("\n\033[1m\033[32mRemoved request (%d, %d) from R%d:\033[0m\n", req1->pickup->id, req1->delivery->id, v1->id);
        printf("R%d: ", v1->id);

        for (Node *node : s.routes[v1].path)
          printf("%d ", node->id);

        printf("(c = %.2f)\n", s.routes[v1].cost);

        printf("\n\033[1m\033[32mRemoved request (%d, %d) from R%d:\033[0m\n", req2->pickup->id, req2->delivery->id, v2->id);
        printf("R%d: ", v2->id);

        for (Node *node : s.routes[v2].path)
          printf("%d ", node->id);

        printf("(c = %.2f)\n", s.routes[v2].cost);
      #endif

      Route best1 = get_cheapest_insertion(req1, s.routes[v2]);
      Route best2 = get_cheapest_insertion(req2, s.routes[v3]);
      Route best3 = get_cheapest_insertion(req3, s.routes[v1]);

      if (!best1.feasible()) {
        // Activate new vehicle to accomodate the request
        Vehicle *v = new Vehicle(s.routes.size() + 1, inst.vehicles[0]->capacity, inst.vehicles[0]->max_route_duration);
        Route r(v);

        r.path.push_back(inst.get_depot());
        r.path.push_back(req1->pickup);
        r.path.push_back(req1->delivery);
        r.path.push_back(inst.get_depot());
        r.evaluate();
        s.add_route(r);
      }
      else {
        s.add_route(best1);
      }

      if (!best2.feasible()) {
        // Activate new vehicle to accomodate the request
        Vehicle *v = new Vehicle(s.routes.size() + 1, inst.vehicles[0]->capacity, inst.vehicles[0]->max_route_duration);
        Route r(v);

        r.path.push_back(inst.get_depot());
        r.path.push_back(req2->pickup);
        r.path.push_back(req2->delivery);
        r.path.push_back(inst.get_depot());
        r.evaluate();
        s.add_route(r);
      }
      else {
        s.add_route(best2);
      }

      if (!best3.feasible()) {
        // Activate new vehicle to accomodate the request
        Vehicle *v = new Vehicle(s.routes.size() + 1, inst.vehicles[0]->capacity, inst.vehicles[0]->max_route_duration);
        Route r(v);

        r.path.push_back(inst.get_depot());
        r.path.push_back(req3->pickup);
        r.path.push_back(req3->delivery);
        r.path.push_back(inst.get_depot());
        r.evaluate();
        s.add_route(r);
      }
      else {
        s.add_route(best3);
      }

      #ifdef DEBUG
        printf("\n\033[1m\033[34mInserting request (%d, %d) in R%d:\033[0m\n", req1->pickup->id, req1->delivery->id, v2->id);

        for (Node *n : best1.path)
          if (n == req1->pickup || n == req1->delivery)
            printf("\033[1m\033[31m%d\033[0m ", n->id);
          else
            printf("%d ", n->id);

        printf("(c = %.2lf)\n", best1.cost);

        printf("\n\033[1m\033[34mInserting request (%d, %d) in R%d:\033[0m\n", req2->pickup->id, req2->delivery->id, v1->id);

        for (Node *n : best2.path)
          if (n == req2->pickup || n == req2->delivery)
            printf("\033[1m\033[31m%d\033[0m ", n->id);
          else
            printf("%d ", n->id);

        printf("(c = %.2lf)\n", best2.cost);
      #endif

      return s;
    }

    Solution two_opt_star(Solution s)
    {
      #ifdef DEBUG
        printf("\n\033[1m\033[33m-> Entering 2-opt* operator...\033[0m\n");
      #endif

      Solution best = s;

      for (std::pair<Vehicle*, Route> p1 : s.routes) {
	      Vehicle *v1 = p1.first;
	      Route r1 = p1.second;

        for (std::pair<Vehicle*, Route> p2 : s.routes) {
	        Vehicle *v2 = p2.first;
	        Route r2 = p2.second;

          if (v1 != v2) {
            int r1_load = 0;

            for (int i = 0; i < r1.path.size() - 1; i++) {
              r1_load += r1.path[i]->load;

              if (r1_load == 0) {
                int r2_load = 0;

                for (int j = 0; j < r2.path.size() - 1; j++) {
                  r2_load += r2.path[j]->load;

                  if (r2_load == 0) {
                    /* There is no point in exchanging segments when they're both immediately after depot
                     * or immediately before depot, since this operation will yield the original routes.
                     */
                    if (!(i == 0 && j == 0) && !(i == r1.path.size() - 2 && j == r2.path.size() - 2)) {
                      Route new_r1(r1.vehicle);
                      Route new_r2(r2.vehicle);

                      new_r1.path.insert(new_r1.path.end(), r1.path.begin(), r1.path.begin() + i + 1);
                      new_r1.path.insert(new_r1.path.end(), r2.path.begin() + j + 1, r2.path.end());

                      new_r2.path.insert(new_r2.path.end(), r2.path.begin(), r2.path.begin() + j + 1);
                      new_r2.path.insert(new_r2.path.end(), r1.path.begin() + i + 1, r1.path.end());

                      // Neighbor solution will be feasible if and only if both routes can be evaluated
                      if (new_r1.evaluate() && new_r2.evaluate()) {
                        Solution neighbor = s;
                        neighbor.add_route(new_r1);
                        neighbor.add_route(new_r2);

                        if (neighbor.cost < best.cost)
                          best = neighbor;
                      }

                      #ifdef DEBUG
                        printf("\nApplying 2-opt* to routes:");
                        printf("\n\tR%d: ", r1.vehicle->id);
                        for (int k = 0; k < r1.path.size(); k++) {
                          printf("%d ", r1.path[k]->id);

                          if (k == i)
                            printf("\033[1m\033[31m|\033[0m ");
                        }

                        printf("\n\tR%d: ", r2.vehicle->id);
                        for (int k = 0; k < r2.path.size(); k++) {
                          printf("%d ", r2.path[k]->id);

                          if (k == j)
                            printf("\033[1m\033[31m|\033[0m ");
                        }

                        printf("\n\n\tR%d': ", new_r1.vehicle->id);
                        for (int k = 0; k < new_r1.path.size(); k++) {
                          printf("%d ", new_r1.path[k]->id);

                          if (k == i)
                            printf("\033[1m\033[34m|\033[0m ");
                        }

                        printf("\n\tR%d': ", new_r2.vehicle->id);
                        for (int k = 0; k < new_r2.path.size(); k++) {
                          printf("%d ", new_r2.path[k]->id);

                          if (k == j)
                            printf("\033[1m\033[34m|\033[0m ");
                        }

                        printf("\n");
                      #endif
                    }
                  }
                }
              }
            }
          }
        }
      }

      return best;
    }

    Solution shift_1_0(Solution s)
    {
      #ifdef DEBUG
        printf("\n\033[1m\033[33m-> Entering shift-1-0 operator...\033[0m\n");
      #endif

      std::pair<Route, Route> best_shift;

      /* Let delta be the gain of shifting a given request from one route to another.
       * We want to find the shift that yields the minimum possible delta value (maximum gain).
       */
      double delta = 0;

      for (std::pair<Vehicle*, Route> pair1 : s.routes) {
	      Vehicle *v1 = pair1.first;
        Route r1 = pair1.second;

        for (std::pair<Vehicle*, Route> pair2 : s.routes) {
	        Vehicle *v2 = pair2.first;
	        Route r2 = pair2.second;

          if (v1 != v2) {
            for (int i = 0; i < r1.path.size(); i++) {
              Node *node = r1.path[i];

              if (node->is_pickup()) {
                Request *req = inst.get_request(node);
                Route new_r2 = get_cheapest_insertion(req, r2);

                if (new_r2.feasible()) {
                  Route new_r1 = r1;
                  new_r1.erase_request(req);

                  double gain = (new_r1.cost + new_r2.cost) - (r1.cost + r2.cost);

                  #ifdef DEBUG
                    printf("\nShifting request (%d, %d) from R%d to R%d\n", req->pickup->id, req->delivery->id, v1->id, v2->id);

                    printf("\tR%d: ", v1->id);
                    for (Node *n : r1.path)
                      if (n == req->pickup || n == req->delivery)
                        printf("\033[1m\033[31m%d\033[0m ", n->id);
                      else
                        printf("%d ", n->id);
                    printf("(c = %.2lf)\n", r1.cost);

                    printf("\tR%d: ", v2->id);
                    for (Node *n : r2.path)
                      printf("%d ", n->id);
                    printf("(c = %.2lf)\n", r2.cost);

                    printf("\n\tR%d': ", v1->id);
                    for (Node *n : new_r1.path)
                        printf("%d ", n->id);
                    printf("(c = %.2lf)\n", new_r1.cost);

                    printf("\tR%d': ", v2->id);
                    for (Node *n : new_r2.path)
                      if (n == req->pickup || n == req->delivery)
                        printf("\033[1m\033[32m%d\033[0m ", n->id);
                      else
                        printf("%d ", n->id);
                    printf("(c = %.2lf)\n", new_r2.cost);

                    if (gain < delta)
                      printf("\n\t\033[1m\033[34mΔf = %.2f\033[0m\n", gain);
                    else
                      printf("\n\tΔf = %.2f\n", gain);
                  #endif

                  if (gain < delta) {
                    new_r1.evaluate(); // Update decision variables
                    best_shift = std::make_pair(new_r1, new_r2);
                    delta = gain;
                  }
                }
              }
            }
          }
        }
      }

      // We found a pair of routes that reduces the total distance of current solution
      if (delta < 0) {
        s.add_route(best_shift.first);
        s.add_route(best_shift.second);
      }

      return s;
    }
  } // namespace reactive_grasp_impl
} // namespace algorithms
