/**
 * @file   Timer.hpp
 * @author Diego Paiva
 * @date   21/05/2019
 *
 * This class is a utility that allows the computation of elapsed time since it's instatiation.
 * Implementation was based in the following article: https://www.learncpp.com/cpp-tutorial/8-16-timing-your-code/
 */

#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include <chrono>

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::duration<double, std::ratio<1>> Seconds;

class Timer
{
private:
  Time::time_point beginning;
public:
  Timer()
  {
    beginning = Time::now();
  }

  ~Timer() { }

  void reset()
  {
    beginning = Time::now();
  }

	double elapsedInSeconds() const
  {
    return std::chrono::duration_cast<Seconds>(Time::now() - beginning).count();
  }
};

#endif // TIMER_H_INCLUDED
