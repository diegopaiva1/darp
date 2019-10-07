/**
 * @file    Point.hpp
 * @author  Diego Paiva e Silva
 * @date    06/10/2019
 */

#ifndef POINT_H_INCLUDED
#define POINT_H_INCLUDED

#include <cmath>

class Point
{
public:
  float x;
  float y;

  Point() {}

  Point(float x, float y)
  {
    this->x = x;
    this->y = y;
  }

  ~Point() {};

  float getDistanceFrom(Point *point)
  {
    return sqrt(pow(this->x - point->x, 2) + pow(this->y - point->y, 2));
  }
};

#endif // POINT_H_INCLUDED
