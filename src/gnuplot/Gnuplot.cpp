/**
 * @file    Gnuplot.cpp
 * @author  Diego Paiva
 * @date    23/10/2019
 */

#include "Gnuplot.hpp"

#include <map>

Singleton *in = Singleton::getInstance();

void Gnuplot::plotSolution(Solution s)
{
  // Clear our gnuplot's tmp folder before saving the resulting plots
  system(("rm -f " + destinationFolder + "*").c_str());

  plotGraph(s.routes, s.cost);

  for (Route r : s.routes)
    plotSchedule(r);
}

void Gnuplot::plotGraph(std::vector<Route> routes, double cost)
{
  // Data to be used in the plot
  std::ofstream dataFile(destinationFolder +  "routes.dat");

  // Header metadata
  dataFile << "# Instance name, Solution cost, Number of routes, Number of requests, Number of stations" << "\n";
  dataFile << in->name << ' ' << cost << ' ' << routes.size() << ' ' <<  in->requestsAmount << ' '
           << in->stationsAmount << "\n";

  // Each datablock must be separated by two line breaks
  dataFile << "\n\n";

  dataFile << "# Id, Latitude, Longitude" << "\n";
  for (Node *node : in->nodes)
    dataFile << node->id << " " << node->latitude << " " << node->longitude << '\n';

  dataFile << "\n\n";

  for (Route r : routes) {
    dataFile << "# x1, y1, x2 - x1, y2 - y1" << "\n";
    for (int i = 0; i < r.path.size() - 1; i++) {
      // We do these calculations because we're going to use gnoplot's 'with vector' directive
      double x1 = r.path[i]->latitude;
      double y1 = r.path[i]->longitude;
      double x2 = r.path[i + 1]->latitude - x1;
      double y2 = r.path[i + 1]->longitude - y1;

      dataFile << x1 << ' ' << y1 << ' ' << x2 << ' ' << y2 << '\n';
    }

    dataFile << "\n\n";
  }

  // Call gnuplot
  popen(("gnuplot " + graphScript).c_str(), "w");
}

void Gnuplot::plotSchedule(Route r)
{
  // Data to be used in the plot
  std::string fileName = "../tmp/gnuplot/schedule" + std::to_string(r.vehicle.id) + ".dat";

  std::ofstream dataFile(fileName, std::ofstream::out | std::ofstream::trunc);
  std::map<int, int> colors;

  // Define the color for each point
  for (int i = 0, color = 0; i < r.path.size(); i++) {
    if (r.path[i]->isPickup()) {
      colors[r.path[i]->id] = color;
      color++;
    }
    else if (r.path[i]->isDelivery()) {
      colors[r.path[i]->id] = colors[r.path[i]->id - in->requestsAmount];
    }
    else {
      colors[r.path[i]->id] = 0;
      color++;
    }
  }

  dataFile << "# A_i, i, color" << '\n';
  for (int i = 0; i < r.arrivalTimes.size(); i++)
    dataFile << r.arrivalTimes[i] << ' ' << i << ' ' << colors[r.path[i]->id] << '\n';

  // Remember that each datablock must be separed by two line breaks
  dataFile << "\n\n";

  dataFile << "# B_i, i, color" << '\n';
  for (int i = 0; i < r.serviceBeginningTimes.size(); i++)
    dataFile << r.serviceBeginningTimes[i] << ' ' << i << ' ' << colors[r.path[i]->id] << '\n';

  dataFile << "\n\n";

  dataFile << "# D_i, i, color" << '\n';
  for (int i = 0; i < r.departureTimes.size() - 1; i++)
    dataFile << r.departureTimes[i]   << ' ' << i << ' ' << colors[r.path[i]->id] << '\n';

  dataFile << "\n\n";

  dataFile << "# D_i,   i" << '\n';
  dataFile << "# A_i+1, i" << '\n';
  for (int i = 0; i < r.departureTimes.size() - 1; i++) {
    dataFile << r.departureTimes[i]   << ' ' << i     << '\n';
    dataFile << r.arrivalTimes[i + 1] << ' ' << i + 1 << '\n';
  }

  dataFile << "\n\n";

  dataFile << "# e_i, i" << '\n';
  dataFile << "# l_i, i" << '\n';
  for (int i = 0; i < r.path.size(); i++) {
    dataFile << r.path[i]->arrivalTime   << ' ' << i << '\n';
    dataFile << r.path[i]->departureTime << ' ' << i << '\n';
  }

  std::string output = destinationFolder + std::to_string(r.vehicle.id) + ".png";

  // Call gnuplot. '-c' flag allows to send command line args to gnuplot
  popen(("gnuplot -c " + scheduleScript + " " + fileName + " " + output + " " + std::to_string(r.path.size())).c_str(), "w");
}
