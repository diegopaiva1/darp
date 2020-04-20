#include "utils/Display.hpp"
#include "fort.hpp"

#include <iomanip>

void Display::printProgress(double bestCost, bool isFeasible, double percentage)
{
  int value        = (int) (percentage * 100);
  int leftPadding  = (int) (percentage * WIDTH);
  int rightPadding = WIDTH - leftPadding;
  auto costColor   = isFeasible ? BOLD_GREEN : BOLD_RED;

  printf(BOLD_WHITE "\rComputing solution... Best found = %s%.2f " BOLD_BLUE "[%.*s%*s] %3d%%" RESET,
         costColor, bestCost, leftPadding, PROGRESSBAR_STRING, rightPadding, "", value);

  fflush(stdout);
}

void Display::printResults(Solution s, double elapsedTime)
{
  for (int k = 0; k < s.routes.size(); k++) {
    fort::char_table table;
    table << fort::header << "#" << "ID" << "A" << "B" << "W" << "D" << "R" << "Z" << "C" << "Q" << "E" << fort::endr;

    for (int i = 0; i < 11; i++)
      table.column(i).set_cell_text_align(fort::text_align::right);

    for (int i = 0; i < s.routes[k].path.size(); i++) {
      table << i
            << s.routes[k].path[i]->id
            << s.routes[k].arrivalTimes[i]
            << s.routes[k].serviceBeginningTimes[i]
            << s.routes[k].waitingTimes[i]
            << s.routes[k].departureTimes[i]
            << s.routes[k].rideTimes[i]
            << s.routes[k].batteryLevels[i]
            << s.routes[k].chargingTimes[i]
            << s.routes[k].load[i]
            << s.routes[k].rideTimeExcesses[i]
            << fort::endr;
    }

    std::cout << '\n' << '\n';
    std::cout << table.to_string() << std::endl;
  }

  printf(BOLD_GREEN "\nBest found = %.2f\nTT \t   = %6.2f\nERT \t   = %5.2f\nTime \t   = %4.2f minutes\n" RESET,
        s.cost, s.travelTime, s.excessRideTime, elapsedTime);
  printf("\nPlots have been saved to tmp/gnuplot directory\n");
}
