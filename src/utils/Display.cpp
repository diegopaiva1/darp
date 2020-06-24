#include "fort.hpp"
#include "utils/Display.hpp"
#include "utils/Gnuplot.hpp"

#include <iostream>

void Display::printProgress(bool feasibility, double obj, double fraction)
{
  int percentage  = (int) (fraction * 100);
  int leftLength  = (int) (fraction * progressBar.size());
  int rightLength = progressBar.size() - leftLength;

  std::cout << std::fixed << std::setprecision(2)
            << BOLD_WHITE
            << "\rComputing solution... Best found = " << (feasibility ? BOLD_GREEN : BOLD_RED) << obj
            << BOLD_BLUE
            << " [" << progressBar.substr(0, leftLength) << std::string(rightLength, ' ') << "] " << percentage << "\%"
            << RESET;

  fflush(stdout);
}

void Display::printRun(Run run)
{
  for (int k = 0; k < run.best.routes.size(); k++) {
    Route r = run.best.routes[k];

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
      int id = r.path[i]->id;
      double e = r.path[i]->arrivalTime;
      double l = r.path[i]->departureTime;
      double A = r.arrivalTimes[i];
      double B = r.serviceBeginningTimes[i];
      double W = r.waitingTimes[i];
      double D = r.departureTimes[i];
      double R = r.rideTimes[i];
      double Z = (r.batteryLevels[i]/r.vehicle->batteryCapacity) * 100.0; // Battery level to be shown in percentage
      double C = r.chargingTimes[i];
      int Q = r.load[i];
      double E = r.rideTimeExcesses[i];

      table << i << id << e << l << A << B << W << D << R << Z << C << Q << E << fort::endr;
    }

    std::cout << '\n' << table.to_string() << '\n';
  }

  std::cout << std::fixed << std::setprecision(2)
            << BOLD_GREEN
            << "Initial    = " << run.bestInit.objFuncValue() << '\n'
            << "Best       = " << run.best.objFuncValue() << '\n'
            << "TT         = " << run.best.travelTime() << '\n'
            << "ERT        = " << run.best.excessRideTime() << '\n'
            << "CPU        = " << run.elapsedMinutes << " min\n"
            << "Seed       = " << run.seed << '\n'
            << "Best it.   = " << run.bestIteration << '\n'
            << "Best alpha = " << run.bestAlpha << '\n';

  std::cout << RESET << "\nPlots have been saved to " << Gnuplot::getDestinationDir() << " directory\n";
}
