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
          << "#" << "ID" << "e" << "l" << "A" << "B" << "W" << "D" << "R" << fort::endr
          << fort::separator;

    table[0][1].set_cell_span(12);
    table[0][1].set_cell_text_align(fort::text_align::center);

    table << std::fixed << std::setprecision(2);

    for (int i = 0; i < 13; i++)
      table.column(i).set_cell_text_align(fort::text_align::right);

    for (int i = 0; i < r.path.size(); i++) {
      int id = r.path[i]->id;
      double e = r.path[i]->arrival_time;
      double l = r.path[i]->departure_time;
      double A = r.arrival_times[i];
      double B = r.service_beginning_times[i];
      double W = r.waiting_times[i];
      double D = r.departure_times[i];
      double R = r.ride_times[i];
      int Q = r.load[i];

      table << i << id << e << l << A << B << W << D << R << Q << fort::endr;
    }

    std::cout << '\n' << table.to_string() << '\n';
  }

  std::cout << std::fixed << std::setprecision(2)
            << BOLD_GREEN
            << "Initial    = " << run.best_init.obj_func_value() << '\n'
            << "Best       = " << run.best.obj_func_value() << '\n'
            << "CPU        = " << run.elapsed_minutes << " min\n"
            << "Seed       = " << run.seed << '\n'
            << "Best it.   = " << run.best_iteration << '\n'
            << "Best alpha = " << run.best_alpha << '\n';

  std::cout << RESET << "\nPlots have been saved to " << Gnuplot::getDestinationDir() << " directory\n";
}
