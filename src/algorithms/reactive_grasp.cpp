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

    Solution best;
    double best_obj = MAXFLOAT;
    Run run;

    // A map to track each alpha performance
    std::map<double, AlphaInfo> alphas_map;

    for (double a : alphas)
      alphas_map[a] = {1.0/alphas.size(), 0.0, 0};

    // Moves to be used within RVND
    std::vector<Move> moves = {};

    if (threads < 1 || threads > omp_get_max_threads())
      threads = omp_get_max_threads();

    double start = omp_get_wtime();

    #pragma omp parallel num_threads(threads)
    {
      // Use std::random_device to generate seed to Random engine to each thread
      unsigned int seed = std::random_device{}();
      Random::seed(1935);

      #pragma omp for
      for (int it = 0; it < iterations; it++) {
        double alpha = get_random_alpha(alphas_map);
        auto init = build_greedy_randomized_solution(alpha);
        bool is_feasible = init != nullptr ? true : false;
        // Solution curr = vnd(*init, moves);

        #pragma omp critical
        if (is_feasible && init->obj_func_value() < best_obj) {
          best = *init;
          best_obj = init->obj_func_value();
          run.best_alpha = alpha;
          run.best_init = *init;
          run.best_iteration = it;
        }

        // Penalize alphas that generated infeasible solutions
        int penalty = !is_feasible ? 10 : 1;

        #pragma omp critical
        {
          alphas_map[alpha].count++;

          if (is_feasible)
            alphas_map[alpha].sum += init->obj_func_value();
          else
            alphas_map[alpha].sum += 2500; // todo: revisar essa constante

          if (it > 0 && it % blocks == 0)
            update_probs(alphas_map, best_obj);
        }

        if (omp_get_thread_num() == 0)
          show_progress(is_feasible, best_obj, (double) it/(iterations/omp_get_num_threads()));
      }
    }

    double finish = omp_get_wtime();

    // Erase any route without requests from best
    for (auto r = best.routes.begin(); r != best.routes.end(); )
      r = r->empty() ? best.routes.erase(r) : r + 1;

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

    std::shared_ptr<Solution> build_greedy_randomized_solution(double alpha)
    {
      auto solution = std::make_shared<Solution>();

      for (Vehicle *&v : inst.vehicles)
        solution->routes.push_back(Route(v));

      for (Route &route : solution->routes) {
        route.path.push_back(inst.get_depot());
        route.path.push_back(inst.get_depot());
      }

      struct Candidate {
        std::shared_ptr<Route> route;
        Request *request;
      };

      std::vector<Candidate> candidates;

      // Initialize candidates
      for (Request *req : inst.requests)
        candidates.push_back({get_cheapest_feasible_insertion(req, *solution), req});

      while (!candidates.empty()) {
        std::sort(candidates.begin(), candidates.end(), [] (Candidate &c1, Candidate &c2) {
          return c1.route->cost < c2.route->cost;
        });

        auto candidate = Random::get(candidates.begin(), candidates.begin() + (int) (alpha * candidates.size()));
        solution->set_route(candidate->route->vehicle, *candidate->route);
        auto selected = candidate->route; // Save route to optimize the update of candidates
        candidates.erase(candidate);

        // Update candidates
        for (Candidate &c : candidates) {
          if (*c.route == *selected) {
            c.route = get_cheapest_feasible_insertion(c.request, *solution);

            if (c.route == nullptr)
              return nullptr;
          }
        }
      }

      return solution;
    }

    std::shared_ptr<Route> get_cheapest_feasible_insertion(Request *req, Solution s)
    {
      std::shared_ptr<Route> best(nullptr);
      double best_cost = MAXFLOAT;

      for (Route r : s.routes) {
        std::shared_ptr<Route> curr = get_cheapest_feasible_insertion(req, r);

        if (curr != nullptr && curr->cost < best_cost) {
          best = curr;
          best_cost = curr->cost;
        }
      }

      return best;
    }

    std::shared_ptr<Route> get_cheapest_feasible_insertion(Request *req, Route r)
    {
      // Best insertion starts with infinity cost, we will update it during the search
      std::shared_ptr<Route> best(nullptr);
      double best_cost = MAXFLOAT;

      // Variables to optimize and "prune" insertion procedure
      double curr_time = r.path[0]->arrival_time;
      int curr_load = r.path[0]->load;

      for (int p = 1; p < r.path.size(); p++) {
        Node *pre = r.path[p - 1];
        Node *suc = r.path[p];

        double earliest_pickup_time = std::max(
          req->pickup->arrival_time, curr_time + pre->service_time + inst.get_travel_time(pre, req->pickup)
        );

        if (earliest_pickup_time < req->pickup->departure_time && curr_load + req->pickup->load <= r.vehicle->capacity) {
          r.path.insert(r.path.begin() + p, req->pickup);

          for (int d = p + 1; d < r.path.size(); d++) {
            r.path.insert(r.path.begin() + d, req->delivery);

            // // Inserting right after the pickup vertex
            // if (d == p + 1) {
            //   if (r.get_total_distance() < best_feasible.cost) {
            //     double earliest_d = std::max(req->delivery->arrival_time, earliest_p + req->pickup->service_time + inst.get_travel_time(req->pickup, req->delivery));
            //     double earliest_suc = std::max(suc->arrival_time, earliest_d + req->delivery->service_time + inst.get_travel_time(req->delivery, suc));

            //     if (earliest_d > req->delivery->departure_time && earliest_suc > suc->departure_time) {
            //       r.path.erase(r.path.begin() + d);
            //       continue;
            //     }
            //   }
            //   else {
            //     r.path.erase(r.path.begin() + d);
            //     continue;
            //   }
            // }
            // else {
            //   bool flag = false;
            //   double last_earliest = earliest_p;

            //   for (int k = p + 1; k <= d; k++) {
            //     Node *nk = r.path[k];
            //     Node *pre_nk = r.path[k - 1];

            //     double earliest_k = std::max(nk->arrival_time, last_earliest + pre_nk->service_time + inst.get_travel_time(pre_nk, nk));

            //     if (earliest_k > nk->departure_time)
            //       flag = true;
            //   }

            //   if (flag || last_earliest - req->delivery->departure_time - req->delivery->service_time > 90.0) {
            //     r.path.erase(r.path.begin() + d);
            //     break;
            //   }

            //   double earliest_after_d = std::max(r.path[d + 1]->arrival_time, last_earliest + req->delivery->service_time + inst.get_travel_time(req->delivery, r.path[d + 1]));

            //   if (earliest_after_d > r.path[d + 1]->departure_time) {
            //     r.path.erase(r.path.begin() + d);
            //     break;
            //   }
            // }

            r.evaluate();

            if (r.feasible() && r.cost < best_cost) {
              best = std::make_shared<Route>(r);
              best_cost = r.cost;
            }

            r.path.erase(r.path.begin() + d);
          }

          r.path.erase(r.path.begin() + p);
        }

        curr_time = std::max(suc->arrival_time, curr_time + pre->service_time + inst.get_travel_time(pre, suc));
        curr_load += suc->load;
      }

      return best;
    }

    Solution vnd(Solution s, std::vector<Move> moves, bool use_randomness)
    {
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
      Solution best = s;
      double best_obj = best.obj_func_value();

      for (Route r : s.routes) {
        for (Node *node : r.path) {
          if (node->is_pickup()) {
            Request *req = inst.get_request(node);
            auto curr = std::make_shared<Route>(r);

            // Erase request by value
            curr->path.erase(std::remove(curr->path.begin(), curr->path.end(), req->pickup),   curr->path.end());
            curr->path.erase(std::remove(curr->path.begin(), curr->path.end(), req->delivery), curr->path.end());

            // Reinsert the request
            curr = get_cheapest_feasible_insertion(req, *curr);

            if (curr != nullptr) {
              // Generate neighbor solution
              Solution neighbor = s;
              neighbor.set_route(curr->vehicle, *curr);
              double neighbor_obj = neighbor.obj_func_value();

              if (neighbor.is_feasible && neighbor_obj < best_obj) {
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

                    new_r1.evaluate();
                    new_r2.evaluate();

                    // Generate neighbor solution
                    Solution neighbor = s;
                    neighbor.set_route(new_r1.vehicle, new_r1);
                    neighbor.set_route(new_r2.vehicle, new_r2);
                    double neighbor_obj = neighbor.obj_func_value();

                    if (neighbor.is_feasible && neighbor_obj < best_obj) {
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
            Request *req = inst.get_request(node);
            auto curr1 = std::make_shared<Route>(r1);

            // Erase request by value
            curr1->path.erase(std::remove(curr1->path.begin(), curr1->path.end(), req->pickup),   curr1->path.end());
            curr1->path.erase(std::remove(curr1->path.begin(), curr1->path.end(), req->delivery), curr1->path.end());

            for (Route r2 : s.routes) {
              if (r1 != r2) {
                // Insert req in the new route
                auto curr2 = get_cheapest_feasible_insertion(req, r2);

                if (curr2 != nullptr) {
                  curr1->evaluate();
                  curr2->evaluate();

                  // Generate neighbor solution;
                  Solution neighbor = s;
                  neighbor.set_route(curr1->vehicle, *curr1);
                  neighbor.set_route(curr2->vehicle, *curr2);
                  double neighbor_obj = neighbor.obj_func_value();

                  if (neighbor.is_feasible && neighbor_obj < best_obj) {
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
