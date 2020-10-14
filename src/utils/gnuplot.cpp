/**
 * @file    Gnuplot.cpp
 * @author  Diego Paiva
 * @date    23/10/2019
 */

#include "utils/gnuplot.hpp"
#include "data-structures/instance.hpp"

#include <thread>   // std::this_thread
#include <fstream>  // std::ofstream
#include <iomanip>  // std::setprecision
#include <iostream> // std::cout

void gnuplot::plot_run(Run run, std::string dir)
{
  if (dir.back() != '/')
    dir.append("/");

  details::plot_solution_graph(run.best_init, dir + "init.png");
  details::plot_solution_graph(run.best, dir + "best.png");

  for (Route r : run.best.routes)
    details::plot_schedule(r, dir + "schedule" + std::to_string(r.vehicle->id) + ".png");

  details::plot_alphas_probs(run.alphas_prob_distribution, dir + "alphas.png");

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

void gnuplot::details::plot_solution_graph(Solution s, std::string output)
{
  const std::string graph_script = "../extras/scripts/gnuplot/graph.gp";

  // Data to be used in the plot
  std::string data_file = std::to_string(s.obj_func_value()) + ".tmp";
  std::ofstream data_stream(data_file, std::ofstream::out | std::ofstream::trunc);

  // Header metadata
  data_stream << "# Instance name, Solution cost, Number of routes, Number of requests" << "\n"
              << inst.name << ' ' << s.obj_func_value() << ' ' << s.routes.size() << ' ' <<  inst.requests.size() << "\n";

  // Each datablock must be separated by two line breaks
  data_stream << "\n\n";

  data_stream << "# Id, Latitude, Longitude" << "\n";
  for (Node *node : inst.nodes)
    data_stream << node->id << " " << node->latitude << " " << node->longitude << '\n';

  data_stream << "\n\n";

  for (Route r : s.routes) {
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

void gnuplot::details::plot_alphas_probs(std::map<double, double> alphas_probs, std::string output)
{
  const std::string alpha_probs_script = "../extras/scripts/gnuplot/alphas.gp";

  // Data to be used in the plot
  std::string data_file = "alphas.tmp";
  std::ofstream data_stream(data_file, std::ofstream::out | std::ofstream::trunc);

  for (auto [alpha, prob] : alphas_probs)
    data_stream << std::fixed << std::setprecision(2) << alpha << " " << prob << '\n';

  call_gnuplot({
    alpha_probs_script,
    data_file,
    output,
    std::to_string(alphas_probs.size()),
    std::to_string(alphas_probs.begin()->first),
    std::to_string(alphas_probs.rbegin()->first)
  });
}
