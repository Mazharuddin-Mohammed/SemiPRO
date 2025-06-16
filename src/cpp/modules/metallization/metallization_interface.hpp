#pragma once
#include "../../core/wafer.hpp"
#include <memory>
#include <string>

class MetallizationInterface {
public:
  virtual ~MetallizationInterface() = default;
  virtual void simulateMetallization(std::shared_ptr<Wafer> wafer, double thickness, const std::string& metal,
                                    const std::string& method) = 0;
};