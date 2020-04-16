#include "Display.hpp"

void Display::printProgress(double bestCost, double percentage)
{
  int value        = (int) (percentage * 100);
  int leftPadding  = (int) (percentage * WIDTH);
  int rightPadding = WIDTH - leftPadding;

  printf(ANSI_COLOR_BOLD ANSI_COLOR_YELLOW "\rComputing solution... "
         ANSI_COLOR_GREEN                  "Best found = %.2f [%.*s%*s] %3d%%"
         ANSI_COLOR_RESET,
         bestCost, leftPadding, PROGRESSBAR_STRING, rightPadding, "", value);

  fflush(stdout);
}

void Display::printResults(Solution s, double elapsedTime)
{
  for (int i = 0; i < s.routes.size(); i++) {
    printf("\n\nRoute %d:\n", i + 1);
    s.routes[i].printSchedule();
  }

  printf(ANSI_COLOR_BOLD ANSI_COLOR_GREEN
        "\nBest found = %.2f\nTT \t   = %6.2f\nERT \t   = %5.2f\nTime \t   = %4.2f minutes\n",
        s.cost, s.travelTime, s.excessRideTime, elapsedTime);
  printf(ANSI_COLOR_RESET "\nPlots have been saved to tmp/gnuplot directory\n");
}
