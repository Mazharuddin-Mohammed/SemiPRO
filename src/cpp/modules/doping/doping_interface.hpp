#ifndef DOPING_INTERFACE_HPP
#define DOPING_INTERFACE_HPP

#include "../../core/wafer.hpp"
#include <memory>

class DopingInterface {
public:
  virtual ~DopingInterface() = default;
  virtual void simulateIonImplantation(std::shared_ptr<Wafer> wafer, double energy, double dose) = 0;
  virtual void simulateDiffusion(std::shared_ptr<Wafer> wafer, double temperature, double time) = 0;
};

#endif // DOPING_INTERFACE_HPP