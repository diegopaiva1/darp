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
    std::vector<Move> moves = {reinsert, shift_1_0, two_opt_star};

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
        Solution curr = vnd(init, moves, true);
        double curr_obj = curr.obj_func_value();

        #pragma omp critical
        if (curr.is_feasible && curr_obj < best_obj) {
          best_obj = curr_obj;
          run.best_init = init;
          run.best = curr;
        }

        #pragma omp critical
        {
          alphas_map[alpha].count++;
          double increase = init.is_feasible ? init.obj_func_value() : 2500; // todo: revisar essa constante
          alphas_map[alpha].sum += increase;
        }

        if (it > 0 && it % blocks == 0) {
          #pragma omp critical
          update_probs(alphas_map, best_obj);
        }

        if (omp_get_thread_num() == 0)
          show_progress(run.best.is_feasible, best_obj, (double) it/(iterations/omp_get_num_threads()));
      }
    }

    run.best_init.delete_empty_routes();
    run.best.delete_empty_routes();

    double finish = omp_get_wtime();

    for (auto [alpha, info] : alphas_map)
      run.alphas_prob_distribution[alpha] = info.probability;

    run.elapsed_minutes = (finish - start)/60;
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
      solution.is_feasible = true;

      for (Vehicle *v : inst.vehicles)
        solution.routes.push_back(Route(v));

      for (Route &route : solution.routes) {
        route.path.push_back(inst.get_depot());
        route.path.push_back(inst.get_depot());
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

        auto selected = Random::get(candidates.begin(), candidates.begin() + (int) (alpha * candidates.size()));
        solution.set_route(selected->route.vehicle, selected->route);

        // Save route pointed by iterator before erasing it, so we can use it to optimize candidates update
        auto selected_route = selected->route;
        candidates.erase(selected);

        // Update candidates
        for (Candidate &c : candidates) {
          if (c.route == selected_route) {
            c.route = get_cheapest_insertion(c.request, solution);

            if (!c.route.is_feasible) {
              solution.is_feasible = false;
              return solution;
            }
          }
        }
      }

      return solution;
    }

    Route get_cheapest_insertion(Request *req, Solution s)
    {
      Route best;
      best.is_feasible = false;
      best.cost = MAXFLOAT;

      for (Route r : s.routes) {
        Route curr = get_cheapest_insertion(req, r);

        if (curr.is_feasible && curr.cost < best.cost)
          best = curr;
      }

      return best;
    }

    Route get_cheapest_insertion(Request *req, Route r)
    {
      Route best;
      best.is_feasible = false;
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

              if (time_gap_between_pickup_delivery <= r.path[p]->max_ride_time && r.evaluate()) {
                best = r;
                best.is_feasible = true;
              }
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
      std::vector<Move> vnd_moves = moves;

      // Only feasible solutions are allowed
      if (!s.is_feasible)
        return s;

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
      Solution best = s;
      double best_obj = best.obj_func_value();

      for (Route r : s.routes) {
        for (Node *node : r.path) {
          if (node->is_pickup()) {
            Request *req = inst.get_request(node);
            Route curr = r;

            // Erase request by value
            curr.path.erase(std::remove(curr.path.begin(), curr.path.end(), req->pickup), curr.path.end());
            curr.path.erase(std::remove(curr.path.begin(), curr.path.end(), req->delivery), curr.path.end());

            // Reinsert the request
            curr = get_cheapest_insertion(req, curr);

            if (curr.is_feasible) {
              Solution neighbor = s;
              neighbor.set_route(curr.vehicle, curr);
              double neighbor_obj = neighbor.obj_func_value();

              if (neighbor_obj < best_obj) {
                best = neighbor;
                best_obj = neighbor_obj;
              }
            }
          }
        }
      }

      return best;
    }

    Solution two_opt_star(Solution s)
    {
      Solution best = s;
      double best_obj = best.obj_func_value();

      for (Route r1 : s.routes) {
        int r1_load = 0;

        for (int i = 1; i < r1.path.size() - 2; i++) {
          r1_load += r1.path[i]->load;

          if (r1_load == 0) {
            for (Route r2 : s.routes) {
              if (r1 != r2) {
                int r2_load = 0;

                for (int j = 1; j < r2.path.size() - 2; j++) {
                  r2_load += r2.path[j]->load;

                  if (r2_load == 0) {
                    Route new_r1(r1.vehicle);
                    Route new_r2(r2.vehicle);

                    // Add first segment of r1 to new_r1
                    for (int k = 0; k <= i; k++)
                      new_r1.path.push_back(r1.path[k]);

                    // Add first segment of r2 to new_r2
                    for (int k = 0; k <= j; k++)
                      new_r2.path.push_back(r2.path[k]);

                    // Add second segment of r2 to new_r1
                    for (int k = j + 1; k < r2.path.size(); k++)
                      new_r1.path.push_back(r2.path[k]);

                    // Add second segment of r1 to new_r2
                    for (int k = i + 1; k < r1.path.size(); k++)
                      new_r2.path.push_back(r1.path[k]);

                    // Neighbor solution will be feasible if and only if both routes can be evaluated to true
                    if (new_r1.evaluate() && new_r2.evaluate()) {
                      Solution neighbor = s;
                      neighbor.set_route(new_r1.vehicle, new_r1);
                      neighbor.set_route(new_r2.vehicle, new_r2);
                      double neighbor_obj = neighbor.obj_func_value();

                      if (neighbor_obj < best_obj) {
                        best = neighbor;
                        best_obj = neighbor_obj;
                      }
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
      Solution best = s;
      double best_obj = best.obj_func_value();

      for (Route r1 : s.routes) {
        for (Node *node : r1.path) {
          if (node->is_pickup()) {
            Request *req = inst.get_request(node);
            Route curr1 = r1;

            // Erase request by value
            curr1.path.erase(std::remove(curr1.path.begin(), curr1.path.end(), req->pickup),   curr1.path.end());
            curr1.path.erase(std::remove(curr1.path.begin(), curr1.path.end(), req->delivery), curr1.path.end());

            for (Route r2 : s.routes) {
              if (r1 != r2) {
                // Insert req in the new route
                Route curr2 = get_cheapest_insertion(req, r2);

                if (curr2.is_feasible) {
                  // TODO: Optimize.
                  // Call evaluate to update cost and schedule even though this route is feasible at this point
                  curr1.evaluate();

                  Solution neighbor = s;
                  neighbor.set_route(curr1.vehicle, curr1);
                  neighbor.set_route(curr2.vehicle, curr2);
                  double neighbor_obj = neighbor.obj_func_value();

                  if (neighbor_obj < best_obj) {
                    best = neighbor;
                    best_obj = neighbor_obj;
                  }
                }
              }
            }
          }
        }
      }

      return best;
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
