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
#include <omp.h>

namespace algorithms
{
  Run reactive_grasp(int iterations, int blocks, std::vector<double> alphas, int threads)
  {
    using namespace reactive_grasp_impl;

    Solution best;
    double best_obj = MAXFLOAT;
    Run run;

    // A map to track each alpha performance
    std::map<double, AlphaInfo> alphas_map;

    for (double a : alphas)
      alphas_map[a] = {1.0/alphas.size(), 0.0, 0};

    // Moves to be used within RVND
    std::vector<Move> moves = {two_opt_star, reinsert, shift_1_0};

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
        Solution curr = rvnd(init, moves);
        double curr_obj = curr.obj_func_value();

        #pragma omp critical
        if ((curr.feasible() && (curr_obj < best_obj || !best.feasible()))) {
          best = curr;
          best_obj = curr_obj;
          run.best_alpha = alpha;
          run.best_init = init;
          run.best_iteration = it;
        }

        // Penalize alphas that generated infeasible solutions
        int penalty = !curr.feasible() ? 10 : 1;

        #pragma omp critical
        {
          alphas_map[alpha].count++;
          alphas_map[alpha].sum += curr_obj * penalty;

          if (it > 0 && it % blocks == 0)
            update_probs(alphas_map, best_obj);
        }

        if (omp_get_thread_num() == 0)
          show_progress(best.feasible(), best_obj, (double) it/(iterations/omp_get_num_threads()));
      }
    }

    double finish = omp_get_wtime();

    // Erase any route without requests from best
    for (auto r = best.routes.begin(); r != best.routes.end(); )
      r = (r->empty()) ? best.routes.erase(r) : r + 1;

    for (auto [alpha, info] : alphas_map)
      run.alphas_prob_distribution[alpha] = info.probability;

    run.best = best;
    run.seed = 0;
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

      for (Vehicle *&v : inst.vehicles)
        solution.routes.push_back(Route(v));

      for (Route &route : solution.routes) {
        route.path.push_back(inst.get_depot());
        route.path.push_back(inst.get_depot());
      }

      struct Candidate {
        Route route;
        Request request;
      };

      std::vector<Candidate> candidates;

      // Initialize candidates
      for (Request &req : inst.requests)
        candidates.push_back({get_cheapest_feasible_insertion(req, solution), req});

      while (!candidates.empty()) {
        std::sort(candidates.begin(), candidates.end(), [] (Candidate &c1, Candidate &c2) {
          return c1.route.cost < c2.route.cost;
        });

        auto candidate = Random::get(candidates.begin(), candidates.begin() + (int) (alpha * candidates.size()));
        solution.set_route(candidate->route.vehicle, candidate->route);
        Route selected = candidate->route; // Save route to optimize the update of candidates
        candidates.erase(candidate);

        // Update candidates
        for (Candidate &c : candidates)
          if (c.route == selected)
            c = {get_cheapest_feasible_insertion(c.request, solution), c.request};
      }

      return solution;
    }

    Route get_cheapest_feasible_insertion(Request req, Solution s)
    {
      Route best_feasible;
      Route best_infeasible;

      best_feasible.cost = MAXFLOAT;
      best_infeasible.cost = MAXFLOAT;

      for (Route r : s.routes) {
        Route curr = get_cheapest_feasible_insertion(req, r);

        if (curr.feasible() && curr.cost < best_feasible.cost)
          best_feasible = curr;
        else if (!curr.feasible() && curr.cost < best_infeasible.cost)
          best_infeasible = curr;
      }

      return best_feasible.cost != MAXFLOAT ? best_feasible : best_infeasible;
    }

    Route get_cheapest_feasible_insertion(Request req, Route r)
    {
      // Best insertion starts with infinity cost, we will update it during the search
      Route best_feasible = r;
      Route best_infeasible = r;

      best_feasible.cost = MAXFLOAT;
      best_infeasible.cost = MAXFLOAT;

      for (int p = 1; p < r.path.size(); p++) {
        r.path.insert(r.path.begin() + p, req.pickup);

        for (int d = p + 1; d < r.path.size(); d++) {
          r.path.insert(r.path.begin() + d, req.delivery);
          r.evaluate();

          if (r.feasible() && r.cost < best_feasible.cost)
            best_feasible = r;
          else if (!r.feasible() && r.cost < best_infeasible.cost)
            best_infeasible = r;

          r.path.erase(r.path.begin() + d);
        }

        r.path.erase(r.path.begin() + p);
      }

      return best_feasible.cost != MAXFLOAT ? best_feasible : best_infeasible;
    }

    Solution rvnd(Solution s, std::vector<Move> moves)
    {
      std::vector<Move> rvnd_moves = moves;

      while (!rvnd_moves.empty()) {
        auto move = Random::get(rvnd_moves); // Get an iterator to a random move
        Solution neighbor = (*move)(s);

        if (neighbor.obj_func_value() < s.obj_func_value()) {
          s = neighbor;
          rvnd_moves = moves;
        }
        else {
          rvnd_moves.erase(move);
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
            Request req = inst.get_request(node);
            Route curr = r;

            // Erase request by value
            curr.path.erase(std::remove(curr.path.begin(), curr.path.end(), req.pickup),   curr.path.end());
            curr.path.erase(std::remove(curr.path.begin(), curr.path.end(), req.delivery), curr.path.end());

            // Reinsert the request
            curr = get_cheapest_feasible_insertion(req, curr);
            curr.evaluate();

            // Generate neighbor solution
            Solution neighbor = s;
            neighbor.set_route(curr.vehicle, curr);
            double neighbor_obj = neighbor.obj_func_value();

            if (neighbor.feasible() && neighbor_obj < best_obj) {
              best = neighbor;
              best_obj = neighbor_obj;
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

                    new_r1.evaluate();
                    new_r2.evaluate();

                    // Generate neighbor solution
                    Solution neighbor = s;
                    neighbor.set_route(new_r1.vehicle, new_r1);
                    neighbor.set_route(new_r2.vehicle, new_r2);
                    double neighbor_obj = neighbor.obj_func_value();

                    if (neighbor.feasible() && neighbor_obj < best_obj) {
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

      return best;
    }

    Solution shift_1_0(Solution s)
    {
      Solution best = s;
      double best_obj = best.obj_func_value();

      for (Route r1 : s.routes) {
        for (Node *node : r1.path) {
          if (node->is_pickup()) {
            Request req = inst.get_request(node);
            Route curr1 = r1;

            // Erase request by value
            curr1.path.erase(std::remove(curr1.path.begin(), curr1.path.end(), req.pickup),   curr1.path.end());
            curr1.path.erase(std::remove(curr1.path.begin(), curr1.path.end(), req.delivery), curr1.path.end());
            curr1.evaluate();

            for (Route r2 : s.routes) {
              if (r1 != r2) {
                // Insert req in the new route
                Route curr2 = get_cheapest_feasible_insertion(req, r2);

                curr2.evaluate();

                // Generate neighbor solution;
                Solution neighbor = s;
                neighbor.set_route(curr1.vehicle, curr1);
                neighbor.set_route(curr2.vehicle, curr2);
                double neighbor_obj = neighbor.obj_func_value();

                if (neighbor.feasible() && neighbor_obj < best_obj) {
                  best = neighbor;
                  best_obj = neighbor_obj;
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
