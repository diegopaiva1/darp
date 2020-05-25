/**
 * @file   Display.hpp
 * @author Diego Paiva
 * @date   16/04/2020
 *
 * Utility to pretty-print the program's progress and results in the console.
 */

#ifndef DISPLAY_HPP_INCLUDED
#define DISPLAY_HPP_INCLUDED

#include "data-structures/Run.hpp"

#include <iostream>

/**
 * @brief Some ANSI-based text style definitions.
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

class Display
{
private:
 /**
  * @brief Create a styled progress bar string of fixed width.
  */
  static inline const std::string progressBar = std::string(60, '#');

public:
 /**
  * @brief Display solution's feasibility and obj. value with a progress bar.
  *
  * @param feasibility Solution's feasibility.
  * @param obj         Solution's obj. value.
  * @param fraction    Fraction of completed iterations.
  */
  static void printProgress(bool feasibility, double obj, double fraction);

 /**
  * @brief Print all data about a Run object. A schedule table containing all decision
  *        variables for every route will be also displayed as follows:
  *
  * +----+----------------------------------------------------------------------------+
  * |  k |                             ROUTE SCHEDULE                                 |
  * +----+----+------+------+------+------+------+------+-----+--------+----+---+-----+
  * |  # | ID |    e |    l |    A |    B |    W |    D |   R |  Z (%) |  C | Q |   E |
  * +----+----+------+------+------+------+------+------+-----+--------+----+---+-----+
  * | ...| ...|   ...|   ...|   ...|   ...|   ...|   ...|  ...|     ...| ...| . |  ...|
  * +----+----+------+------+------+------+------+------+-----+--------+----+---+-----+
  *
  * where 'k' is the id of the route.
  *
  * @param run A Run.
  */
  static void printRun(Run run);
};

#endif // DISPLAY_HPP_INCLUDED
