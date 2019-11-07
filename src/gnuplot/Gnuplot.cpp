/**
 * @file    Gnuplot.cpp
 * @author  Diego Paiva
 * @date    23/10/2019
 */

#include "Gnuplot.hpp"

Singleton *i = Singleton::getInstance();

void Gnuplot::plotSolution(Solution s)
{
  // File where solution info will be stored
  std::ofstream plotData("../tmp/gnuplot/solution.dat");

  plotData << i->name << ' ' << s.cost << ' ' << s.routes.size() << ' ' <<  i->requestsAmount << ' '
           << i->stationsAmount << "\n\n\n";

  for (Node* node : i->nodes)
    plotData << node->id << " " << node->latitude << " " << node->longitude << '\n';

  plotData << "\n\n";

  for (Route route : s.routes) {
    for (int i = 0; i < route.path.size() - 1; i++) {
      float x1 = route.path[i]->latitude;
      float y1 = route.path[i]->longitude;
      float x2 = route.path[i + 1]->latitude - x1;
      float y2 = route.path[i + 1]->longitude - y1;

      plotData << x1 << ' ' << y1 << ' ' << x2 << ' ' << y2 << '\n';
    }

    plotData << "\n\n";
  }

  std::ifstream gnuplotCommandsFile("../src/gnuplot/solution.gp");

 /* Opens an interface that one can use to send commands as if they were typing into the gnuplot
  * command line. One may add "-persistent" to keep the plot open after the program terminates.
  */
  FILE *gnuplotPipe = popen("gnuplot", "w");

  std::string currLine;

  while (std::getline(gnuplotCommandsFile, currLine))
    fprintf(gnuplotPipe, "%s \n", currLine.c_str());
}
