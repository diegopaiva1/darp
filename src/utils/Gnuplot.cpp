/**
 * @file    Gnuplot.cpp
 * @author  Diego Paiva
 * @date    23/10/2019
 */

#include "utils/Gnuplot.hpp"
#include "data-structures/instance.hpp"

#include <thread>  // std::this_thread
#include <fstream> // std::ofstream
#include <algorithm>
#include <iomanip>

void Gnuplot::plotRun(Run run)
{
  // Clear destination dir before saving plots
  system(("rm -f " + destinationDir + "*").c_str());

  plotGraph(run.best_init.routes, run.best_init.obj_func_value(), destinationDir + "init.png");
  plotGraph(run.best.routes, run.best.obj_func_value(), destinationDir + "best.png");

  for (int k = 0; k < run.best.routes.size(); k++)
    plotSchedule(run.best.routes[k], destinationDir + "schedule" + std::to_string(k + 1) + ".png");

  plotAlphasProbabilityDistribution(run.alphas_prob_distribution, destinationDir + "alphas.png");

  // Sleep to avoid concurrence issues with gnuplot process
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Erase all .tmp files
  system(("rm -f " + destinationDir + "/*.tmp").c_str());
}

void Gnuplot::plotGraph(std::vector<Route> routes, double cost, std::string output)
{
  // Data to be used in the plot
  std::string dataFile = destinationDir + std::to_string(cost) + ".tmp";

  std::ofstream dataStream(dataFile, std::ofstream::out | std::ofstream::trunc);

  // Header metadata
  dataStream << "# Instance name, Solution cost, Number of routes, Number of requests, Number of stations" << "\n";
  dataStream << inst.name << ' ' << cost << ' ' << routes.size() << ' ' <<  inst.requests_num << ' '
             /* << inst.stationsAmount TODO: APAGAR stationsAmount*/ <<   "\n";

  // Each datablock must be separated by two line breaks
  dataStream << "\n\n";

  dataStream << "# Id, Latitude, Longitude" << "\n";
  for (Node *node : inst.nodes)
    dataStream << node->id << " " << node->latitude << " " << node->longitude << '\n';

  dataStream << "\n\n";

  for (Route r : routes) {
    dataStream << "# x1, y1, x2 - x1, y2 - y1" << "\n";

    for (int i = 0; i < r.path.size() - 1; i++) {
      // We do these calculations because we're going to use gnuplot's 'with vector' directive
      double x1 = r.path[i]->latitude;
      double y1 = r.path[i]->longitude;
      double x2 = r.path[i + 1]->latitude - x1;
      double y2 = r.path[i + 1]->longitude - y1;

      dataStream << x1 << ' ' << y1 << ' ' << x2 << ' ' << y2 << '\n';
    }

    dataStream << "\n\n";
  }

  // Call gnuplot. '-c' flag allows to send command line args to gnuplot
  auto arg1 = dataFile;
  auto arg2 = output;

  popen(("gnuplot -c " + graphScript + " " + arg1 + " " + arg2).c_str(), "w");
}

void Gnuplot::plotSchedule(Route r, std::string output)
{
  // Data to be used in the plot
  std::string dataFile = destinationDir + "schedule" + std::to_string(r.vehicle->id) + ".tmp";

  std::ofstream dataStream(dataFile, std::ofstream::out | std::ofstream::trunc);

  std::map<int, int> colors;

  // Define the color for each point
  for (int i = 0, color = 0; i < r.path.size(); i++) {
    if (r.path[i]->is_pickup()) {
      colors[r.path[i]->id] = color;
      color++;
    }
    else if (r.path[i]->is_delivery()) {
      colors[r.path[i]->id] = colors[r.path[i]->id - inst.requests_num];
    }
    else {
      colors[r.path[i]->id] = 0;
      color++;
    }
  }

  dataStream << "# A_i, i, color" << '\n';
  for (int i = 0; i < r.arrival_times.size(); i++)
    dataStream << r.arrival_times[i] << ' ' << i << ' ' << colors[r.path[i]->id] << '\n';

  // Remember that each datablock must be separed by two line breaks
  dataStream << "\n\n";

  dataStream << "# B_i, i, color" << '\n';
  for (int i = 0; i < r.service_beginning_times.size(); i++)
    dataStream << r.service_beginning_times[i] << ' ' << i << ' ' << colors[r.path[i]->id] << '\n';

  dataStream << "\n\n";

  dataStream << "# D_i, i, color" << '\n';
  for (int i = 0; i < r.departure_times.size() - 1; i++)
    dataStream << r.departure_times[i]   << ' ' << i << ' ' << colors[r.path[i]->id] << '\n';

  dataStream << "\n\n";

  dataStream << "# D_i,   i" << '\n';
  dataStream << "# A_i+1, i" << '\n';
  for (int i = 0; i < r.departure_times.size() - 1; i++) {
    dataStream << r.departure_times[i]   << ' ' << i     << '\n';
    dataStream << r.arrival_times[i + 1] << ' ' << i + 1 << '\n';
  }

  dataStream << "\n\n";

  dataStream << "# e_i, i" << '\n';
  dataStream << "# l_i, i" << '\n';
  for (int i = 0; i < r.path.size(); i++) {
    dataStream << r.path[i]->arrival_time   << ' ' << i << '\n';
    dataStream << r.path[i]->departure_time << ' ' << i << '\n';
  }

  // Call gnuplot. '-c' flag allows to send command line args to gnuplot
  auto arg1 = dataFile;
  auto arg2 = output;
  auto arg3 = std::to_string(r.path.size());

  popen(("gnuplot -c " + scheduleScript + " " + arg1 + " " + arg2 + " " + arg3).c_str(), "w");
}

void Gnuplot::plotAlphasProbabilityDistribution(std::map<double, double> alphaWithProbs, std::string output)
{
  // Data to be used in the plot
  std::string dataFile = destinationDir + "alphas.tmp";

  std::ofstream dataStream(dataFile, std::ofstream::out | std::ofstream::trunc);

  for (auto [alpha, prob] : alphaWithProbs)
    dataStream << std::fixed << std::setprecision(2) << alpha << " " << prob << '\n';

  // Call gnuplot. '-c' flag allows to send command line args to gnuplot
  auto arg1 = dataFile;
  auto arg2 = output;
  auto arg3 = std::to_string(alphaWithProbs.size());
  auto arg4 = std::to_string(alphaWithProbs.begin()->first);
  auto arg5 = std::to_string(alphaWithProbs.rbegin()->first);

  popen(("gnuplot -c " + alphaScript + " " + arg1 + " " + arg2 + " " + arg3 + " " + arg4 + " " + arg5).c_str(), "w");
}

const std::string Gnuplot::getDestinationDir()
{
  return destinationDir;
}
