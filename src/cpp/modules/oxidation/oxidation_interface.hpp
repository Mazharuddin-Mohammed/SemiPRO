#ifndef OXIDATION_INTERFACE_HPP
#define OXIDATION_INTERFACE_HPP

#include "../../core/wafer.hpp"
#include <memory>

class OxidationInterface {
public:
  virtual ~OxidationInterface() = default;
  virtual void simulateOxidation(std::shared_ptr<Wafer> wafer, double temperature, double time) = 0;
};

#endif // OXIDATION_INTERFACE_HPP