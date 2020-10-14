#include "fort.hpp"
#include "utils/display.hpp"

#include <iostream> // std::cout

void display::show_progress(bool feasibility, double obj_func_value, double fraction)
{
  std::string progress_bar = std::string(60, '#');
  int percentage = (int) (fraction * 100);
  int left_length = (int) (fraction * progress_bar.size());
  int right_length = progress_bar.size() - left_length;

  std::cout << std::fixed << std::setprecision(2)
            << BOLD_WHITE
            << "\rComputing solution... Best found = " << (feasibility ? BOLD_GREEN : BOLD_RED) << obj_func_value
            << BOLD_BLUE
            << " [" << progress_bar.substr(0, left_length) << std::string(right_length, ' ') << "] " << percentage << "\%"
            << RESET;
  fflush(stdout);
}

void display::show_run(Run run)
{
  // Show schedule of every route
  for (Route &r : run.best.routes) {
    fort::char_table table;
    int cols = 10;

    // Decision variables
    table << fort::header << r.vehicle->id << "ROUTE SCHEDULE" << fort::endr
          << "#" << "ID" << "e" << "l" << "A" << "B" << "W" << "D" << "R" << "Q" << fort::endr << fort::separator;

    table[0][1].set_cell_span(cols - 1);
    table[0][1].set_cell_text_align(fort::text_align::center);
    table << std::fixed << std::setprecision(2);

    for (int i = 0; i < cols; i++)
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

  // Show run info
  std::cout << std::fixed << std::setprecision(2)
            << BOLD_GREEN
            << "Initial    = " << run.best_init.obj_func_value() << '\n'
            << "Best       = " << run.best.obj_func_value() << '\n'
            << "CPU        = " << run.elapsed_minutes << " min\n"
            << "Seed       = " << run.seed << '\n'
            << "Best it.   = " << run.best_iteration << '\n'
            << "Best alpha = " << run.best_alpha << '\n';
}
