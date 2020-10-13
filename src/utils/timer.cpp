/**
 * @file   timer.cpp
 * @author Diego Paiva
 * @date   21/05/2019
 */

#include "utils/timer.hpp"

Timer::Timer()
{
  beginning = Time::now();
}

void Timer::reset()
{
  beginning = Time::now();
}

double Timer::elapsed_seconds() const
{
  return std::chrono::duration_cast<Seconds>(Time::now() - beginning).count();
}

double Timer::elapsed_minutes() const
{
  return std::chrono::duration_cast<Minutes>(Time::now() - beginning).count();
}

double Timer::elapsed_hours() const
{
  return std::chrono::duration_cast<Hours>(Time::now() - beginning).count();
}
