/**
 * @file   timer.hpp
 * @author Diego Paiva
 * @date   21/05/2019
 *
 * This class is a utility that allows the computation of elapsed time since it's instatiation.
 * Implementation based in the article https://www.learncpp.com/cpp-tutorial/8-16-timing-your-code/
 */

#ifndef TIMER_HPP_INCLUDED
#define TIMER_HPP_INCLUDED

#include <chrono>

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::duration<double, std::ratio<1>> Seconds;
typedef std::chrono::duration<double, std::ratio<60>> Minutes;
typedef std::chrono::duration<double, std::ratio<3600>> Hours;

class Timer
{
private:
  Time::time_point beginning;

public:
  Timer();

  ~Timer() {};

  void reset();

	double elapsed_seconds() const;

  double elapsed_minutes() const;

  double elapsed_hours() const;
};

#endif // TIMER_HPP_INCLUDED