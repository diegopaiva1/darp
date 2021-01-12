/**
 * @file   main.cpp
 * @author Diego Paiva
 * @date   24/09/2019
 */

#include "algorithms/grasp.hpp"
#include "instance.hpp"
#include "json.hpp"

#include <ctime>   // std::chrono
#include <fstream> // std::ofstream
#include <iomanip> // std::setprecision, std::setw
#include <sstream> // std::stringstream

// Register computation start date
std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

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
    fprintf(stderr, "Usage: %s <instance> <runs> <threads> <output json name>\n", argv[0]);
    return EXIT_FAILURE;
  }

  inst.init(argv[1]);

  std::vector<Run> runs;
  int num_runs = std::stoi(argv[2]);

  for (int i = 1; i <= num_runs; i++) {
    Run run = algorithms::grasp(2048, 0.85, std::stoi(argv[3]));

    printf(
      "Run %d of %d ......... [c = %.2f, t = %.2fs]\n", i, num_runs, run.best.cost, run.elapsed_seconds
    );

    runs.push_back(run);
  }

  to_json(runs, argv[4]);
  return EXIT_SUCCESS;
}

void to_json(std::vector<Run> runs, std::string file_name)
{
  nlohmann::ordered_json j;
  double best_cost, mean_cost = 0.0, mean_cpu = 0.0, standard_deviation = 0.0;

  for (int i = 0; i < runs.size(); i++) {
    double value = runs[i].best.cost;

    mean_cost += value;
    mean_cpu += runs[i].elapsed_seconds;

    {
      std::stringstream ss;
      ss << std::setprecision(2) << std::fixed << runs[i].best_init.cost;
      ss >> j["runs"][std::to_string(i + 1)]["init"];
    }

    {
      std::stringstream ss;
      ss << std::setprecision(2) << std::fixed << runs[i].best.cost;
      ss >> j["runs"][std::to_string(i + 1)]["best"];
    }

    {
      std::stringstream ss;
      ss << std::setprecision(2) << std::fixed << runs[i].best.routes.size();
      ss >> j["runs"][std::to_string(i + 1)]["vehicles"];
    }

    {
      std::stringstream ss;
      ss << std::setprecision(2) << std::fixed << runs[i].elapsed_seconds;
      ss >> j["runs"][std::to_string(i + 1)]["cpu_time_in_seconds"];
    }

    j["runs"][std::to_string(i + 1)]["feasible"] = runs[i].best.feasible();
    j["runs"][std::to_string(i + 1)]["threads"] = runs[i].seeds.size();
    j["runs"][std::to_string(i + 1)]["seeds"] = runs[i].seeds;

    if (i == 0 || value < best_cost) {
      j["best_run"] = i + 1;
      best_cost = value;
    }
  }

  mean_cost /= runs.size();
  mean_cpu /= runs.size();

  for (int i = 0; i < runs.size(); i++)
    standard_deviation += pow(runs[i].best.cost - mean_cost, 2);

  if (runs.size() > 1)
    standard_deviation = sqrt(standard_deviation/(runs.size() - 1));

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

  j["start_date"] = std::ctime(&now);
  now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  j["end_date"] = std::ctime(&now);

  // Write prettified JSON to another file
  std::ofstream out(file_name);
  out << std::setw(4) << j << std::endl;
}
