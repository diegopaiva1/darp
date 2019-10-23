/**
 * @file    Plotter.hpp
 * @author  Diego Paiva
 * @date    23/10/2019
 *
 * A classe Plotter utiliza a interface de linha de comando provida pelo Gnuplot® para realizar
 * a plotagem de uma instância do DARP (objeto da classe 'Instance') e da solução para uma instância.
 */

#ifndef PLOTTER_HPP_INCLUDED
#define PLOTTER_HPP_INCLUDED

#include <iostream>
#include <regex>

#include "../data-structures/Singleton.hpp"
#include "../data-structures/Solution.hpp"

class Plotter
{
public:
  Plotter();

  ~Plotter();

  static void plotSolution(Solution s);

private:
  static void sendCommandToGnuplot(FILE *file, const char* command);
};

#endif // PLOTTER_HPP_INCLUDED
