/**
 * @file    gnuplot.cpp
 * @author  Diego Paiva
 * @date    23/10/2019
 */

#include "gnuplot.hpp"
#include "instance.hpp"

#include <algorithm> // std::sort
#include <fstream>   // std::ofstream
#include <iomanip>   // std::setprecision
#include <iostream>  // std::cout
#include <thread>    // std::this_thread
#include <sstream>   // std::stringstream

void gnuplot::plot_run(Run run, std::string dir)
{
  if (dir.back() != '/')
    dir.append("/");

  details::plot_solution_graph(run.best, dir + "best.png");
  details::plot_solution_graph(run.init, dir + "init.png");

  for (auto pair : run.best.routes)
    details::plot_schedule(pair.second, dir + "schedule" + std::to_string(pair.first->id) + ".png");

  // Sleep to avoid concurrence issues with gnuplot process
  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  std::cout << "\nPlots have been stored in " << dir << " directory" << '\n';

  // Erase all .tmp files
  system(("rm *.tmp"));
}

void gnuplot::details::call_gnuplot(std::vector<std::string> args)
{
  // '-c' flag allows to send command line args to gnuplot
  std::string cmd = "gnuplot -c";

  for (std::string arg : args)
    cmd.append(" " + arg);

  popen(cmd.c_str(), "w");
}

void gnuplot::plot_convergence_analysis(std::unordered_map<std::string, std::vector<Run>> runs, std::string output)
{
  const std::string script = "../extras/scripts/gnuplot/convergence_analysis.gp";

  // Data to be used in the plot
  std::string data_file = "conv-" + inst.name + ".dat";
  std::ofstream data_stream(data_file, std::ofstream::out | std::ofstream::trunc);

  for (Run run : runs["grasp"]) {
    for (auto pair : run.convergence)
      data_stream << pair.first << ' ' << pair.second << '\n';

    data_stream << "\n\n";
  }

  for (Run run : runs["ils"]) {
    for (auto pair : run.convergence)
      data_stream << pair.first << ' ' << pair.second << '\n';

    data_stream << "\n\n";
  }

  details::call_gnuplot({script, data_file, output});
}

void gnuplot::tttplot(std::unordered_map<std::string, std::vector<Run>> runs, double target, std::string output)
{
  const std::string script = "../extras/scripts/gnuplot/tttplot.gp";

  // Data to be used in the plot
  std::string data_file = "tttplot-" + inst.name + ".dat";
  std::ofstream data_stream(data_file, std::ofstream::out | std::ofstream::trunc);
  std::vector<double> grasp_times_to_target;
  std::vector<double> ils_times_to_target;

  for (Run run : runs["grasp"])
    grasp_times_to_target.push_back(run.elapsed_seconds);

  for (Run run : runs["ils"])
    ils_times_to_target.push_back(run.elapsed_seconds);

  std::sort(grasp_times_to_target.begin(), grasp_times_to_target.end());
  std::sort(ils_times_to_target.begin(), ils_times_to_target.end());

  for (int i = 0; i < grasp_times_to_target.size(); i++)
    data_stream << i + 1 << ' ' << grasp_times_to_target[i] << '\n';

  data_stream << "\n\n";

  for (int i = 0; i < ils_times_to_target.size(); i++)
    data_stream << i + 1 << ' ' << ils_times_to_target[i] << '\n';

  std::stringstream target_ss;
  target_ss << std::fixed << std::setprecision(2) << target;

  details::call_gnuplot({script, data_file, target_ss.str(), output});
}

void gnuplot::details::plot_solution_graph(Solution s, std::string output)
{
  const std::string graph_script = "../extras/scripts/gnuplot/graph.gp";

  // Data to be used in the plot
  std::string data_file = std::to_string(s.cost) + ".tmp";
  std::ofstream data_stream(data_file, std::ofstream::out | std::ofstream::trunc);

  // Header metadata
  data_stream << "# Instance name, Solution cost, Number of routes, Number of requests" << "\n"
              << inst.name << ' ' << s.cost << ' ' << s.routes.size() << ' ' <<  inst.requests.size() << "\n";

  // Each datablock must be separated by two line breaks
  data_stream << "\n\n";

  data_stream << "# Id, Latitude, Longitude" << "\n";
  for (Node *node : inst.nodes)
    data_stream << node->id << " " << node->latitude << " " << node->longitude << '\n';

  data_stream << "\n\n";

  for (auto pair : s.routes) {
    Route r = pair.second;
    data_stream << "# x1, y1, x2 - x1, y2 - y1" << "\n";

    for (int i = 0; i < r.path.size() - 1; i++) {
      // Perform these calculations to use gnuplot's 'with vector' directive
      double x1 = r.path[i]->latitude;
      double y1 = r.path[i]->longitude;
      double x2 = r.path[i + 1]->latitude - x1;
      double y2 = r.path[i + 1]->longitude - y1;

      data_stream << x1 << ' ' << y1 << ' ' << x2 << ' ' << y2 << '\n';
    }

    data_stream << "\n\n";
  }

  call_gnuplot({graph_script, data_file, output});
}

void gnuplot::details::plot_schedule(Route r, std::string output)
{
  const std::string schedule_script = "../extras/scripts/gnuplot/schedule.gp";

  // Data to be used in the plot
  std::string data_file = "schedule" + std::to_string(r.vehicle->id) + ".tmp";
  std::ofstream data_stream(data_file, std::ofstream::out | std::ofstream::trunc);

  std::map<int, int> colors;

  // Define the color for each point
  for (int i = 0, color = 0; i < r.path.size(); i++) {
    if (r.path[i]->is_pickup()) {
      colors[r.path[i]->id] = color;
      color++;
    }
    else if (r.path[i]->is_delivery()) {
      colors[r.path[i]->id] = colors[r.path[i]->id - inst.requests.size()];
    }
    else {
      colors[r.path[i]->id] = 0;
      color++;
    }
  }

  data_stream << "# A_i, i, color" << '\n';
  for (int i = 0; i < r.arrival_times.size(); i++)
    data_stream << r.arrival_times[i] << ' ' << i << ' ' << colors[r.path[i]->id] << '\n';

  // Remember that each datablock must be separed by two line breaks
  data_stream << "\n\n";

  data_stream << "# B_i, i, color" << '\n';
  for (int i = 0; i < r.service_beginning_times.size(); i++)
    data_stream << r.service_beginning_times[i] << ' ' << i << ' ' << colors[r.path[i]->id] << '\n';

  data_stream << "\n\n";

  data_stream << "# D_i, i, color" << '\n';
  for (int i = 0; i < r.departure_times.size() - 1; i++)
    data_stream << r.departure_times[i]   << ' ' << i << ' ' << colors[r.path[i]->id] << '\n';

  data_stream << "\n\n";

  data_stream << "# D_i,   i" << '\n';
  data_stream << "# A_i+1, i" << '\n';
  for (int i = 0; i < r.departure_times.size() - 1; i++) {
    data_stream << r.departure_times[i]   << ' ' << i     << '\n';
    data_stream << r.arrival_times[i + 1] << ' ' << i + 1 << '\n';
  }

  data_stream << "\n\n";

  data_stream << "# e_i, i" << '\n';
  data_stream << "# l_i, i" << '\n';
  for (int i = 0; i < r.path.size(); i++) {
    data_stream << r.path[i]->arrival_time   << ' ' << i << '\n';
    data_stream << r.path[i]->departure_time << ' ' << i << '\n';
  }

  call_gnuplot({schedule_script, data_file, output, std::to_string(r.path.size())});
}
