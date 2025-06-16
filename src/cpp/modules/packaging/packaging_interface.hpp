#pragma once
#include "../../core/wafer.hpp"
#include <memory>
#include <string>

class PackagingInterface {
public:
  virtual ~PackagingInterface() = default;
  virtual void simulatePackaging(std::shared_ptr<Wafer> wafer, double substrate_thickness,
                                const std::string& substrate_material, int num_wires) = 0;
  virtual void performElectricalTest(std::shared_ptr<Wafer> wafer, const std::string& test_type) = 0;
};