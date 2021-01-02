/**
 * @file    main.cpp
 * @author  Diego Paiva
 * @date    24/09/2019
 */

#include "instance.hpp"
#include "json.hpp"
#include "algorithms/reactive_grasp.hpp"

#include <iostream> // std::cerr, std::cout
#include <iomanip>  // std::setprecision, std::setw
#include <fstream>  // std::ofstream

/*
 * Save execution information and statistics as JSON file.
 *
 * @param runs      A std::vector containing all runs.
 * @param file_name Name of the output file.
 */
void to_json(std::vector<Run> runs, std::string file_name);

int main(const int argc, const char* argv[])
{
  const int min_args = 4, max_args = 4, args_given = argc - 1;

  if (args_given < min_args || args_given > max_args) {
    std::cerr << "Usage: " << argv[0] << " <instance> <runs> <threads> <output file name>\n";
    return EXIT_FAILURE;
  }

  inst.init(argv[1]);

  std::vector<Run> runs;

  for (int i = 0; i < std::stoi(argv[2]); i++) {
    Run run = algorithms::reactive_grasp(
      1600, 160, {0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 1.0}, std::stoi(argv[3])
    );

    runs.push_back(run);
  }

  to_json(runs, argv[4]);

  return EXIT_SUCCESS;
}

void to_json(std::vector<Run> runs, std::string file_name)
{
  nlohmann::json j;
  double best_cost = MAXFLOAT, mean_cost, mean_cpu, standard_deviation = 0.0;

  for (int i = 0; i < runs.size(); i++) {
    double value = runs[i].best.obj_func_value();

    if (value < best_cost) {
      j["best_run"] = i + 1;
      best_cost = value;
    }

    mean_cost += value;
    mean_cpu += runs[i].elapsed_seconds;

    {
      std::stringstream ss;
      ss << std::setprecision(2) << std::fixed << runs[i].best_init.obj_func_value();
      ss >> j["runs"][std::to_string(i + 1)]["init"];
    }

    {
      std::stringstream ss;
      ss << std::setprecision(2) << std::fixed << runs[i].best.obj_func_value();
      ss >> j["runs"][std::to_string(i + 1)]["best"];
    }

    {
      std::stringstream ss;
      ss << std::setprecision(2) << std::fixed << runs[i].best.routes.size();
      ss >> j["runs"][std::to_string(i + 1)]["vehicles"];
    }

    j["runs"][std::to_string(i + 1)]["feasible"] = runs[i].best.feasible();

    for (auto [alpha, prob] : runs[i].alphas_prob_distribution) {
      std::stringstream ss1;
      std::stringstream ss2;

      ss1 << std::setprecision(2) << std::fixed << alpha;
      ss2 << std::setprecision(2) << std::fixed << prob;

      ss2 >> j["runs"][std::to_string(i + 1)]["alphas_probability_distribution"][ss1.str()];
    }

    j["runs"][std::to_string(i + 1)]["threads"] = runs[i].seeds.size();
    j["runs"][std::to_string(i + 1)]["seeds"] = runs[i].seeds;
  }

  mean_cost /= runs.size();
  mean_cpu /= runs.size();

  if (runs.size() > 1) {
    for (int i = 0; i < runs.size(); i++)
      standard_deviation += pow(runs[i].best.obj_func_value() - mean_cost, 2);

    standard_deviation = sqrt(standard_deviation/(runs.size() - 1));
  }

  {
    std::stringstream ss;
    ss << std::setprecision(2) << std::fixed << best_cost;
    ss >> j["best_cost"];
  }

  {
    std::stringstream ss;
    ss << std::setprecision(2) << std::fixed << mean_cost;
    ss >> j["mean_cost"];
  }

  {
    std::stringstream ss;
    ss << std::setprecision(2) << std::fixed << standard_deviation;
    ss >> j["cost_standard_deviation"];
  }

  {
    std::stringstream ss;
    ss << std::setprecision(2) << std::fixed << mean_cpu;
    ss >> j["mean_cpu_time_in_seconds"];
  }

  // Write prettified JSON to another file
  std::ofstream out(file_name);
  out << std::setw(4) << j << std::endl;
}
