/**
 * @file   Timer.hpp
 * @author Diego Paiva
 * @date   21/05/2019
 *
 * This class is a utility that allows the computation of elapsed time since it's instatiation.
 * Implementation was based in the following article: https://www.learncpp.com/cpp-tutorial/8-16-timing-your-code/
 */

#include "utils/Timer.hpp"

Timer::Timer()
{
  beginning = Time::now();
}

Timer::~Timer()
{
  // Empty destructor
}

void Timer::reset()
{
  beginning = Time::now();
}

double Timer::elapsedInSeconds() const
{
  return std::chrono::duration_cast<Seconds>(Time::now() - beginning).count();
}

double Timer::elapsedInMinutes() const
{
  return std::chrono::duration_cast<Minutes>(Time::now() - beginning).count();
}

double Timer::elapsedInHours() const
{
  return std::chrono::duration_cast<Hours>(Time::now() - beginning).count();
}
