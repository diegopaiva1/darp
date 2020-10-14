/**
 * @file   display.hpp
 * @author Diego Paiva
 * @date   16/04/2020
 *
 * Utility to pretty-print program's progress and results in the console.
 */

#ifndef DISPLAY_HPP_INCLUDED
#define DISPLAY_HPP_INCLUDED

#include "data-structures/run.hpp"

/**
 * Some ANSI-based text style definitions.
 */
#define RESET        "\033[0m"
#define BLACK        "\033[30m"
#define RED          "\033[31m"
#define GREEN        "\033[32m"
#define YELLOW       "\033[33m"
#define BLUE         "\033[34m"
#define MAGENTA      "\033[35m"
#define CYAN         "\033[36m"
#define WHITE        "\033[37m"
#define BOLD_BLACK   "\033[1m\033[30m"
#define BOLD_RED     "\033[1m\033[31m"
#define BOLD_GREEN   "\033[1m\033[32m"
#define BOLD_YELLOW  "\033[1m\033[33m"
#define BOLD_BLUE    "\033[1m\033[34m"
#define BOLD_MAGENTA "\033[1m\033[35m"
#define BOLD_CYAN    "\033[1m\033[36m"
#define BOLD_WHITE   "\033[1m\033[37m"

namespace display
{
 /**
  * Show solution's feasibility and obj. value within a progress bar.
  *
  * @param feasibility    Solution's feasibility.
  * @param obj_func_value Solution's obj. func. value.
  * @param fraction       Fraction of completed iterations.
  */
  void show_progress(bool feasibility, double obj_func_value, double fraction);

 /**
  * Show all info regarding a Run object. A schedule table containing all decision
  * variables for every route will also be displayed.
  *
  * @param run A Run object.
  */
  void show_run(Run run);
}

#endif // DISPLAY_HPP_INCLUDED
