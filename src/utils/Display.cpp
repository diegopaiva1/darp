#include "utils/Display.hpp"
#include "fort.hpp"

#include <iostream>
#include <iomanip>

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
    Route r = s.routes[k];

    fort::char_table table;

    table << fort::header << k + 1 << "ROUTE SCHEDULE" << fort::endr
          << "#" << "ID" << "e" << "l" << "A" << "B" << "W" << "D" << "R" << "Z (%)" << "C" << "Q" << "E" << fort::endr
          << fort::separator;

    table[0][1].set_cell_span(12);
    table[0][1].set_cell_text_align(fort::text_align::center);

    table << std::fixed << std::setprecision(2);

    for (int i = 0; i < 13; i++)
      table.column(i).set_cell_text_align(fort::text_align::right);

    for (int i = 0; i < r.path.size(); i++) {
      int   id = r.path[i]->id;
      double e = r.path[i]->arrivalTime;
      double l = r.path[i]->departureTime;
      double A = r.arrivalTimes[i];
      double B = r.serviceBeginningTimes[i];
      double W = r.waitingTimes[i];
      double D = r.departureTimes[i];
      double R = r.rideTimes[i];
      double Z = (r.batteryLevels[i]/r.vehicle.batteryCapacity) * 100.0; // Battery level to be shown in percentage
      double C = r.chargingTimes[i];
      int    Q = r.load[i];
      double E = r.rideTimeExcesses[i];

      table << i << id << e << l << A << B << W << D << R << Z << C << Q << E << fort::endr;
    }

    std::cout << '\n' << table.to_string() << '\n';
  }

  printf(BOLD_GREEN "Best found = %.2f\nTT \t   = %6.2f\nERT \t   = %5.2f\nTime \t   = %4.2f minutes\n" RESET,
        s.cost, s.travelTime, s.excessRideTime, elapsedTime);
  printf("\nPlots have been saved to tmp/gnuplot directory\n");
}
