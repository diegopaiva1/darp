/**
 * @file   Display.hpp
 * @author Diego Paiva
 * @date   16/04/2020
 *
 * Utility to pretty-print the program's progress and results in the console.
 */

#ifndef DISPLAY_HPP_INCLUDED
#define DISPLAY_HPP_INCLUDED

#include "data-structures/Solution.hpp"

/**
 * @brief The progress bar style and width while running the program.
 */
#define PROGRESSBAR_STRING "############################################################"
#define WIDTH 60

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
public:
 /**
  * @brief Display solution's cost and feasibility and a progress bar.
  *
  * @param solution   A Solution.
  * @param percentage Fraction of completed iterations.
  */
  static void printProgress(Solution s, double percentage);

 /**
  * @brief Print a schedule table containing all decision variables for every route in solution 's'.
  *        Table is displayed as follows:
  *
  *  +----+-----------------------------------------------------------------+
  *  | x  |                       ROUTE SCHEDULE                            |
  *  +----+-----+------+------+------+------+------+------+-----+-----+-----+
  *  | #  |   ID|     A|     B|     W|     D|     R|     Z|    C|    Q|    E|
  *  +----+-----+------+------+------+------+------+------+-----+-----+-----+
  *  | 0  |  ...|   ...|   ...|   ...|   ...|   ...|   ...| ... | ... |  ...|
  *  +----+-----+------+------+------+------+------+------+-----+-----+-----+
  *
  * where 'x' is the id of the route. Then print solution's cost and elapsed time.
  *
  * @param s           The Solution.
  * @param elapsedTime Time spent to achieve the Solution.
  */
  static void printSolutionInfoWithElapsedTime(Solution s, double elapsedTime);
};

#endif // DISPLAY_HPP_INCLUDED
