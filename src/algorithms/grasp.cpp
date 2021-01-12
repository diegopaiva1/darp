/**
 * @file   reactive_grasp.cpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "algorithms/grasp.hpp"
#include "instance.hpp"
#include "gnuplot.hpp"

#include <iostream> // std::cout
#include <iomanip>  // std::setprecision
#include <cfloat>   // FLT_MAX
#include <omp.h>    // OpenMP

namespace algorithms
{
  Run grasp(int iterations, double random_param, int thread_count)
  {
    using namespace details;

    if (thread_count < 1 || thread_count > omp_get_max_threads())
      thread_count = omp_get_max_threads();

    Run run;
    run.best.cost = FLT_MAX;
    std::vector<Move> moves = {reinsert, two_opt_star, shift_1_0};

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

        Solution curr = vnd(init, moves);

        #pragma omp critical
        if (curr.feasible() && curr.cost < run.best.cost) {
          run.best = curr;
          run.best_init = init;
        }
      }
    }

    run.best.delete_empty_routes();
    run.best_init.delete_empty_routes();

    double finish = omp_get_wtime();
    run.elapsed_seconds = finish - start;

    return run;
  }

  /******************************************/
  /* GRASP implementations details */
  /******************************************/
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

          r.cost = inst.get_travel_time(inst.get_depot(), chosen_candidate->request->pickup) +
                   inst.get_travel_time(chosen_candidate->request->pickup, chosen_candidate->request->delivery) +
                   inst.get_travel_time(chosen_candidate->request->delivery, inst.get_depot());

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
        r.insert_node(req->pickup, p);

        for (int i = p - 1; i < r.path.size(); i++) {
          r.compute_earliest_time(i);
          r.compute_load(i);
        }

        if ((r.earliest_times[p] < r.path[p]->departure_time) && (r.load[p - 1] + r.path[p]->load <= r.vehicle->capacity)) {
          for (int d = p + 1; d < r.path.size(); d++) {
            r.insert_node(req->delivery, d);

            for (int i = d - 1; i < r.path.size(); i++) {
              r.compute_earliest_time(i);
              r.compute_load(i);
            }

            if (r.cost < best.cost) {
              for (int i = p + 1; i <= d + 1; i++) {
                if ((r.earliest_times[i] > r.path[i]->departure_time) || (r.load[i - 1] + r.path[i]->load > r.vehicle->capacity))
                  // Violation found
                  break;
              }

              double time_gap_between_pickup_delivery = r.earliest_times[d] - r.path[p]->departure_time - r.path[p]->service_time;

              if (time_gap_between_pickup_delivery <= r.path[p]->max_ride_time && r.evaluate())
                best = r;
            }

            r.erase_node(d);
          }
        }

        r.erase_node(p);
      }

      return best;
    }

    Solution vnd(Solution s, std::vector<Move> moves, bool use_randomness)
    {
      // Only feasible solutions are allowed
      if (!s.feasible())
        return s;

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
              int delivery_index;

              for (int j = i; j < r.path.size(); j++) {
                if (r.path[j] == req->delivery) {
                  delivery_index = j;
                  break;
                }
              }

              curr.erase_node(delivery_index);
              curr.erase_node(i);

              // Reinsert the request
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
        for (Node *node : routes.begin()->second.path)
          if (node->is_pickup())
            unplanned.push_back(inst.get_request(node));

        s.routes.erase(routes.begin()->first);
      }

      while (!unplanned.empty()) {
        auto request = Random::get(unplanned);
        Route best;
        double min_increase = FLT_MAX;

        #ifdef DEBUG
          printf("\n\tSelected request (%d, %d):\n", (*request)->pickup->id, (*request)->delivery->id);
        #endif

        for (std::pair<Vehicle*, Route> pair : s.routes) {
          Vehicle *v = pair.first;
	        Route r = pair.second;
          Route test = get_cheapest_insertion(*request, r);

          if (test.cost - r.cost < min_increase) {
            min_increase = test.cost - r.cost;
            best = test;
          }

          #ifdef DEBUG
            printf("\t\tR%d: ", v->id);
            for (Node *n : test.path)
              if (n == (*request)->pickup || n == (*request)->delivery)
                printf("\033[1m\033[32m%d\033[0m ", n->id);
              else
                printf("%d ", n->id);

            printf("(Δf = %.2lf)\n", test.cost - r.cost);
          #endif
        }

        if (min_increase == FLT_MAX) {
          #ifdef DEBUG
            printf("\n\t\t-> No feasible insertion found! Solution will remain infeasible.\n");
          #endif

          // Add a null vehicle to make solution infeasible again
          s.add_route(Route(nullptr));
          return s;
        }

        s.add_route(best);
        unplanned.erase(request);

        #ifdef DEBUG
          printf("\n\t\t-> Selected route R%d with Δf = %.2f\n", best.vehicle->id, min_increase);
        #endif
      }

      #ifdef DEBUG
        printf("\n\t\033[1m\033[32mSolution repaired successfully!\033[0m\n");
      #endif

      return s;
    }

    Solution two_opt_star(Solution s)
    {
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
                Route new_r1 = r1;
                Route new_r2 = get_cheapest_insertion(req, r2);
                int delivery_index;

                for (int j = i; j < r1.path.size(); j++) {
                  if (r1.path[j] == req->delivery) {
                    delivery_index = j;
                    break;
                  }
                }

                new_r1.erase_node(delivery_index);
                new_r1.erase_node(i);

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
                  best_shift = std::make_pair(new_r1, new_r2);
                  delta = gain;
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
