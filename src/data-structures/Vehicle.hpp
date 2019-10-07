#ifndef VEHICLE_H_INCLUDED
#define VEHICLE_H_INCLUDED

class Vehicle
{
public:
  int id;
  int capacity;
  float dischargingRate;
  float initialBatteryLevel;
  float batteryCapacity;
  float minFinalBatteryRatioLevel;

  Vehicle() {}

  Vehicle(int id) { this->id = id; }

  Vehicle(int id, int capacity)
  {
    this->id = id;
    this->capacity = capacity;
  }

  ~Vehicle() {}
};

#endif // VEHICLE_H_INCLUDED
