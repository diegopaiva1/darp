#include "utils/Display.hpp"
#include "fort.hpp"

void Display::printProgress(Solution s, double percentage)
{
  int value        = (int) (percentage * 100);
  int leftPadding  = (int) (percentage * WIDTH);
  int rightPadding = WIDTH - leftPadding;
  auto costColor   = s.isFeasible() ? BOLD_GREEN : BOLD_RED;

  printf(BOLD_WHITE "\rComputing solution... Best found = %s%.2f " BOLD_BLUE "[%.*s%*s] %3d%%" RESET,
         costColor, s.cost, leftPadding, PROGRESSBAR_STRING, rightPadding, "", value);

  fflush(stdout);
}

void Display::printSolutionInfoWithElapsedTime(Solution s, double elapsedTime)
{
  for (int k = 0; k < s.routes.size(); k++) {
    fort::char_table table;

    table << fort::header << k + 1 << "ROUTE SCHEDULE" << fort::endr
          << "#" << "ID" << "A" << "B" << "W" << "D" << "R" << "Z" << "C" << "Q" << "E" << fort::endr
          << fort::separator;

    table[0][1].set_cell_span(10);
    table[0][1].set_cell_text_align(fort::text_align::center);

    for (int i = 1; i < 11; i++)
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

    std::cout << '\n' << table.to_string() << std::endl;
  }

  printf(BOLD_GREEN "Best found = %.2f\nTT \t   = %6.2f\nERT \t   = %5.2f\nTime \t   = %4.2f minutes\n" RESET,
        s.cost, s.travelTime, s.excessRideTime, elapsedTime);
  printf("\nPlots have been saved to tmp/gnuplot directory\n");
}
