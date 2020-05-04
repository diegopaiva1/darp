/**
 * @file    Gnuplot.cpp
 * @author  Diego Paiva
 * @date    23/10/2019
 */

#include "utils/Gnuplot.hpp"
#include "data-structures/Singleton.hpp"

#include <map>     // std::map
#include <thread>  // std::this_thread
#include <fstream> // std::ofstream

void Gnuplot::plotSolution(Solution s)
{
  // Clear destination dir before saving plots
  system(("rm -f " + destinationDir + "*").c_str());

  plotGraph(s.routes, s.cost, destinationDir + "graph.png");

  for (int k = 0; k < s.routes.size(); k++)
    plotSchedule(s.routes[k], destinationDir + "schedule" + std::to_string(k + 1) + ".png");

  // Sleep to avoid concurrence issues with gnuplot process
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  // Erase all .tmp files
  system(("rm -f " + destinationDir + "/*.tmp").c_str());
}

void Gnuplot::plotGraph(std::vector<Route> routes, double cost, std::string output)
{
  // Data to be used in the plot
  std::string dataFile = destinationDir + "routes.tmp";

  std::ofstream dataStream(dataFile, std::ofstream::out | std::ofstream::trunc);

  // Header metadata
  dataStream << "# Instance name, Solution cost, Number of routes, Number of requests, Number of stations" << "\n";
  dataStream << inst->name << ' ' << cost << ' ' << routes.size() << ' ' <<  inst->requestsAmount << ' '
             << inst->stationsAmount << "\n";

  // Each datablock must be separated by two line breaks
  dataStream << "\n\n";

  dataStream << "# Id, Latitude, Longitude" << "\n";
  for (Node *node : inst->nodes)
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
  std::string dataFile = destinationDir + "schedule" + std::to_string(r.vehicle.id) + ".tmp";

  std::ofstream dataStream(dataFile, std::ofstream::out | std::ofstream::trunc);

  std::map<int, int> colors;

  // Define the color for each point
  for (int i = 0, color = 0; i < r.path.size(); i++) {
    if (r.path[i]->isPickup()) {
      colors[r.path[i]->id] = color;
      color++;
    }
    else if (r.path[i]->isDelivery()) {
      colors[r.path[i]->id] = colors[r.path[i]->id - inst->requestsAmount];
    }
    else {
      colors[r.path[i]->id] = 0;
      color++;
    }
  }

  dataStream << "# A_i, i, color" << '\n';
  for (int i = 0; i < r.arrivalTimes.size(); i++)
    dataStream << r.arrivalTimes[i] << ' ' << i << ' ' << colors[r.path[i]->id] << '\n';

  // Remember that each datablock must be separed by two line breaks
  dataStream << "\n\n";

  dataStream << "# B_i, i, color" << '\n';
  for (int i = 0; i < r.serviceBeginningTimes.size(); i++)
    dataStream << r.serviceBeginningTimes[i] << ' ' << i << ' ' << colors[r.path[i]->id] << '\n';

  dataStream << "\n\n";

  dataStream << "# D_i, i, color" << '\n';
  for (int i = 0; i < r.departureTimes.size() - 1; i++)
    dataStream << r.departureTimes[i]   << ' ' << i << ' ' << colors[r.path[i]->id] << '\n';

  dataStream << "\n\n";

  dataStream << "# D_i,   i" << '\n';
  dataStream << "# A_i+1, i" << '\n';
  for (int i = 0; i < r.departureTimes.size() - 1; i++) {
    dataStream << r.departureTimes[i]   << ' ' << i     << '\n';
    dataStream << r.arrivalTimes[i + 1] << ' ' << i + 1 << '\n';
  }

  dataStream << "\n\n";

  dataStream << "# e_i, i" << '\n';
  dataStream << "# l_i, i" << '\n';
  for (int i = 0; i < r.path.size(); i++) {
    dataStream << r.path[i]->arrivalTime   << ' ' << i << '\n';
    dataStream << r.path[i]->departureTime << ' ' << i << '\n';
  }

  // Call gnuplot. '-c' flag allows to send command line args to gnuplot
  auto arg1 = dataFile;
  auto arg2 = output;
  auto arg3 = std::to_string(r.path.size());

  popen(("gnuplot -c " + scheduleScript + " " + arg1 + " " + arg2 + " " + arg3).c_str(), "w");
}

const std::string Gnuplot::getDestinationDir()
{
  return destinationDir;
}
