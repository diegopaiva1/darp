/**
 * @file    Plotter.hpp
 * @author  Diego Paiva
 * @date    23/10/2019
 */

#include "Plotter.hpp"

Singleton *i = Singleton::getInstance();

Plotter::Plotter()
{
  // Empty constructor
}

Plotter::~Plotter()
{
  // Empty destructor
}

void Plotter::plotSolution(Solution s)
{
  const char* pointsFile = "points.temp";
  const char* routesFile = "routes.temp";
  const char* gnuplotCommandsFileName = "gnuplot-commands.gp";

  // Temp file where each node id and coordinates will be written
  FILE *points = fopen(pointsFile, "w");

  for (Node* node : i->nodes)
    fprintf(points, "%d %f %f \n", node->id, node->latitude, node->longitude);

  // Temp file where each route will be written
  FILE *routes = fopen(routesFile, "w");

  fprintf(routes, "%d\n", s.routes.size());

  for (Route *route : s.routes) {
    for (Node *node : route->path)
      fprintf(routes, "%d ", node->id);
    fprintf(routes, "\n");
  }

  std::fstream gnuplotCommandsFile(gnuplotCommandsFileName, std::ios::in | std::ios::out);

 /* Abre uma interface para enviar comandos como se estivesse digitando na linha de comando do gnuplot.
  * A flag -persistent mantem o plot aberto mesmo depois de o programa encerrar
  */
  FILE *gnuplotPipe = popen("gnuplot -persistent", "w");

  std::string currLine;

  while (std::getline(gnuplotCommandsFile, currLine))
    sendCommandToGnuplot(gnuplotPipe, currLine.c_str());
}

void Plotter::sendCommandToGnuplot(FILE *file, const char* command)
{
  fprintf(file, "%s \n", command);
}
