/**
 * @file   reactive_grasp.cpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "algorithms/reactive_grasp.hpp"
#include "instance.hpp"
#include "gnuplot.hpp"

#include <iostream> // std::cout
#include <iomanip>  // std::setprecision
#include <omp.h>    // OpenMP

namespace algorithms
{
  Run reactive_grasp(int iterations, int blocks, std::vector<double> alphas, int threads)
  {
    using namespace reactive_grasp_impl;

    Run run;
    double best_obj = MAXFLOAT;

    // A map to track each alpha performance
    std::map<double, AlphaInfo> alphas_map;

    for (double a : alphas)
      alphas_map[a] = {1.0/alphas.size(), 0.0, 0};

    // Moves to be used within RVND
    std::vector<Move> moves = {two_opt_star, shift_1_0, reinsert};

    if (threads < 1 || threads > omp_get_max_threads())
      threads = omp_get_max_threads();

    double start = omp_get_wtime();

    #pragma omp parallel num_threads(threads)
    {
      // Use std::random_device to generate seed to Random engine to each thread
      unsigned int seed = std::random_device{}();
      Random::seed(seed);

      #pragma omp for
      for (int it = 0; it < iterations; it++) {
        double alpha = get_random_alpha(alphas_map);
        Solution init = build_greedy_randomized_solution(alpha);

        if (!init.feasible())
          init = repair(init);

        Solution curr = vnd(init, moves, true);
        double curr_obj = curr.obj_func_value();

        #pragma omp critical
        if (curr.feasible() && curr_obj < best_obj) {
          best_obj = curr_obj;
          run.best_init = init;
          run.best = curr;
        }

        #pragma omp critical
        {
          alphas_map[alpha].count++;
          int penalty = init.feasible() ? 1 : init.routes.size();
          alphas_map[alpha].sum += init.obj_func_value() * penalty;
        }

        if (it > 0 && it % blocks == 0) {
          #pragma omp critical
          update_probs(alphas_map, best_obj);
        }

        if (omp_get_thread_num() == 0)
          show_progress(run.best.feasible(), best_obj, (double) it/(iterations/omp_get_num_threads()));
      }
    }

    run.best_init.delete_empty_routes();
    run.best.delete_empty_routes();

    double finish = omp_get_wtime();

    for (auto [alpha, info] : alphas_map)
      run.alphas_prob_distribution[alpha] = info.probability;

    run.elapsed_seconds = finish - start;
    gnuplot::plot_run(run, "../data/plots/");

    return run;
  }

  /******************************************/
  /* Reactive GRASP implementations details */
  /******************************************/
  namespace reactive_grasp_impl
  {
    double get_random_alpha(std::map<double, AlphaInfo> alphas_map)
    {
      double rand = Random::get(0.0, 1.0);
      double sum = 0.0;

      for (auto [alpha, info] : alphas_map) {
        sum += info.probability;

        if (rand <= sum)
          return alpha;
      }

      return 0;
    }

    Solution build_greedy_randomized_solution(double alpha)
    {
      Solution solution;

      for (Vehicle *v : inst.vehicles) {
        Route r = Route(v);
        r.path.push_back(inst.get_depot());
        r.path.push_back(inst.get_depot());
        solution.add_route(v, r);
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
        std::sort(candidates.begin(), candidates.end(), [] (Candidate &c1, Candidate &c2) {
          return c1.route.cost < c2.route.cost;
        });

        auto chosen_candidate = Random::get(candidates.begin(), candidates.begin() + (int) (alpha * candidates.size()));

        if (chosen_candidate->route.feasible()) {
          solution.add_route(chosen_candidate->route.vehicle, chosen_candidate->route);
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

          solution.add_route(v, r);
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
      double delta = best.cost = MAXFLOAT;

      for (auto [v, r] : s.routes) {
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
      best.cost = MAXFLOAT;

      for (int p = 1; p < r.path.size(); p++) {
        r.path.insert(r.path.begin() + p, req->pickup);

        for (int i = p - 1; i < r.path.size(); i++) {
          r.compute_earliest_time(i);
          r.compute_load(i);
        }

        if ((r.earliest_times[p] < r.path[p]->departure_time) && (r.load[p - 1] + r.path[p]->load <= r.vehicle->capacity)) {
          for (int d = p + 1; d < r.path.size(); d++) {
            r.path.insert(r.path.begin() + d, req->delivery);

            for (int i = d - 1; i < r.path.size(); i++) {
              r.compute_earliest_time(i);
              r.compute_load(i);
            }

            // TODO: Otimizar para O(1)
            if (r.get_total_distance() < best.cost) {
              for (int i = p + 1; i <= d + 1; i++) {
                if ((r.earliest_times[i] > r.path[i]->departure_time) || (r.load[i - 1] + r.path[i]->load > r.vehicle->capacity))
                  // Violation found
                  break;
              }

              double time_gap_between_pickup_delivery = r.earliest_times[d] - r.path[p]->departure_time - r.path[p]->service_time;

              if (time_gap_between_pickup_delivery <= r.path[p]->max_ride_time && r.evaluate())
                best = r;
            }

            r.path.erase(r.path.begin() + d);
          }
        }

        r.path.erase(r.path.begin() + p);
      }

      return best;
    }

    Solution vnd(Solution s, std::vector<Move> moves, bool use_randomness)
    {
      // Only feasible solutions are allowed
      if (!s.feasible())
        return s;

      std::vector<Move> vnd_moves = moves;

      while (!vnd_moves.empty()) {
        auto move = use_randomness ? Random::get(vnd_moves) : vnd_moves.begin();
        Solution neighbor = (*move)(s);

        if (neighbor.obj_func_value() < s.obj_func_value()) {
          s = neighbor;
          vnd_moves = moves;
        }
        else {
          vnd_moves.erase(move);
        }
      }

      return s;
    }

    Solution reinsert(Solution s)
    {
      #ifdef DEBUG
        printf("\n\033[1m\033[33m-> Entering reinsert operator...\033[0m\n");
      #endif

      for (auto [v, r] : s.routes) {
        Route best_reinsertion = r;

        #ifdef DEBUG
          printf("\n<<<<<< Initial cost of route %d = %.2f >>>>>>\n", best_reinsertion.vehicle->id, best_reinsertion.cost);
        #endif

        // Perform reinsert only in routes with more than one request accommodated
        if (r.path.size() > 4) {
          for (Node *node : r.path) {
            if (node->is_pickup()) {
              Request *req = inst.get_request(node);
              Route curr = r;

              // Erase request by value
              curr.path.erase(std::remove(curr.path.begin(), curr.path.end(), req->pickup), curr.path.end());
              curr.path.erase(std::remove(curr.path.begin(), curr.path.end(), req->delivery), curr.path.end());

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

        s.add_route(best_reinsertion.vehicle, best_reinsertion);
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

      std::sort(routes.begin(), routes.end(), [] (auto &p1, auto &p2) {
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
        double min_increase = MAXFLOAT;

        #ifdef DEBUG
          printf("\n\tSelected request (%d, %d):\n", (*request)->pickup->id, (*request)->delivery->id);
        #endif

        for (auto [v, r] : s.routes) {
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

        if (min_increase == MAXFLOAT) {
          #ifdef DEBUG
            printf("\n\t\t-> No feasible insertion found! Solution will remain infeasible.\n");
          #endif

          // Add a null vehicle to make solution infeasible again
          s.add_route(nullptr, Route());
          return s;
        }

        s.add_route(best.vehicle, best);
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
      double best_obj = best.obj_func_value();

      for (auto [v1, r1] : s.routes) {
        for (auto [v2, r2] : s.routes) {
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
                        neighbor.add_route(new_r1.vehicle, new_r1);
                        neighbor.add_route(new_r2.vehicle, new_r2);
                        double neighbor_obj = neighbor.obj_func_value();

                        if (neighbor_obj < best_obj) {
                          best = neighbor;
                          best_obj = neighbor_obj;
                        }
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

      for (auto [v1, r1] : s.routes) {
        for (auto [v2, r2] : s.routes) {
          if (v1 != v2) {
            for (Node *node : r1.path) {
              if (node->is_pickup()) {
                Request *req = inst.get_request(node);
                Route new_r1 = r1;
                Route new_r2 = get_cheapest_insertion(req, r2);

                // Erase request by value
                new_r1.path.erase(std::remove(new_r1.path.begin(), new_r1.path.end(), req->pickup), new_r1.path.end());
                new_r1.path.erase(std::remove(new_r1.path.begin(), new_r1.path.end(), req->delivery), new_r1.path.end());
                new_r1.evaluate(); // TODO: Optimize?

                double gain = (new_r1.get_total_distance() + new_r2.cost) - (r1.cost + r2.cost);

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
        s.add_route(best_shift.first.vehicle, best_shift.first);
        s.add_route(best_shift.second.vehicle, best_shift.second);
      }

      return s;
    }

    void update_probs(std::map<double, AlphaInfo> &alphas_map, double best_cost)
    {
      double q_sum = 0.0;

      for (auto [alpha, info] : alphas_map)
        if (info.avg() > 0)
          q_sum += best_cost/info.avg();

      for (auto &[alpha, info] : alphas_map)
        info.probability = (best_cost/info.avg())/q_sum;
    }

    void show_progress(bool feasibility, double obj_func_value, double fraction)
    {
      // Some ANSI-based text style definitions
      std::string bold_red = "\033[1m\033[31m";
      std::string bold_green = "\033[1m\033[32m";
      std::string bold_blue = "\033[1m\033[34m";
      std::string bold_white = "\033[1m\033[37m";
      std::string reset = "\033[0m";

      std::string progress_bar = std::string(60, '#');
      int percentage = (int) (fraction * 100);
      int left_length = (int) (fraction * progress_bar.size());
      int right_length = progress_bar.size() - left_length;

      std::cout << std::fixed << std::setprecision(2)
                << bold_white
                << "\rComputing solution... Best found = " << (feasibility ? bold_green : bold_red) << obj_func_value
                << bold_blue
                << " [" << progress_bar.substr(0, left_length) << std::string(right_length, ' ') << "] "
                << percentage << "\%"
                << reset;

      fflush(stdout);
    }
  } // namespace reactive_grasp_impl
} // namespace algorithms
