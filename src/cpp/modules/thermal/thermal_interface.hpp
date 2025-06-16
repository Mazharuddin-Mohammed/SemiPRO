#pragma once
#include "../../core/wafer.hpp"
#include <memory>

class ThermalInterface {
public:
  virtual ~ThermalInterface() = default;
  virtual void simulateThermal(std::shared_ptr<Wafer> wafer, double ambient_temperature, double current) = 0;
};