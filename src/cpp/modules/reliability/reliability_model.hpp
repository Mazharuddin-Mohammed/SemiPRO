#pragma once
#include "reliability_interface.hpp"
#include "../../core/utils.hpp"
#include <Eigen/Dense>

class ReliabilityModel : public ReliabilityInterface {
public:
  ReliabilityModel();
  void performReliabilityTest(std::shared_ptr<Wafer> wafer, double current, double voltage) override;

private:
  void computeElectromigration(std::shared_ptr<Wafer> wafer, double current);
  void computeThermalStress(std::shared_ptr<Wafer> wafer);
  void computeDielectricBreakdown(std::shared_ptr<Wafer> wafer, double voltage);
};