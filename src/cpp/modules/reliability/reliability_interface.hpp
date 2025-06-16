#pragma once
#include "../../core/wafer.hpp"
#include <memory>

class ReliabilityInterface {
public:
  virtual ~ReliabilityInterface() = default;
  virtual void performReliabilityTest(std::shared_ptr<Wafer> wafer, double current, double voltage) = 0;
};