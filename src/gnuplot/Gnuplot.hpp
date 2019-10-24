/**
 * @file    Gnuplot.hpp
 * @author  Diego Paiva
 * @date    23/10/2019
 *
 * A classe Gnuplot utiliza a interface de linha de comando provida pelo Gnuplot® para realizar
 * a plotagem de uma instância do DARP (objeto da classe 'Instance') e da solução para uma instância.
 */

#ifndef GNUPLOT_HPP_INCLUDED
#define GNUPLOT_HPP_INCLUDED

#include <fstream>
#include <iostream>

#include "../data-structures/Singleton.hpp"
#include "../data-structures/Solution.hpp"

class Gnuplot
{
public:
  static void plotSolution(Solution s);
};

#endif // GNUPLOT_HPP_INCLUDED
