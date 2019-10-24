/**
 * @file    Gnuplot.cpp
 * @author  Diego Paiva
 * @date    23/10/2019
 */

#include "Gnuplot.hpp"

void Gnuplot::plotSolution(Solution s)
{
  // Temp file where each node id and coordinates will be written
  std::ofstream points("../tmp/gnuplot/points.dat");

  points << Singleton::getInstance()->name << '\n';

  for (Node* node : Singleton::getInstance()->nodes)
    points << node->id << " " << node->latitude << " " << node->longitude << '\n';

  // Temp file where each node id and coordinates will be written
  std::ofstream routes("../tmp/gnuplot/routes.dat");

  routes << "# " << Singleton::getInstance()->name << '\n';
  routes << s.routes.size() << '\n';

  for (Route *route : s.routes) {
    for (Node *node : route->path)
      routes << node->id << " ";

    routes << '\n';
  }

  std::ifstream gnuplotCommandsFile("../src/gnuplot/solution.gp");

 /* Abre uma interface para enviar comandos como se estivesse digitando na linha de comando do gnuplot.
  * A flag -persistent mantem o plot aberto mesmo depois de o programa encerrar
  */
  FILE *gnuplotPipe = popen("gnuplot -persistent", "w");

  std::string currLine;

  while (std::getline(gnuplotCommandsFile, currLine))
    fprintf(gnuplotPipe, "%s \n", currLine.c_str());
}
