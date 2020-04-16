/**
 * @file   Display.hpp
 * @author Diego Paiva
 * @date   16/04/2020
 *
 * Utility to pretty-print the program's progress in the console.
 */

#ifndef DISPLAY_H_INCLUDED
#define DISPLAY_H_INCLUDED

#include "data-structures/Solution.hpp"

#define PROGRESSBAR_STRING "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define WIDTH 60
#define ANSI_COLOR_BOLD    "\e[1m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\e[0m\x1b[0m"

class Display
{
public:
  static void printProgress(double bestCost, double percentage);

  static void printResults(Solution s, double elapsedTime);
};

#endif // DISPLAY_H_INCLUDED
