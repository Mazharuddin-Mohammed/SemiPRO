#pragma once
#include "packaging_interface.hpp"
#include "../../core/utils.hpp"
#include <Eigen/Dense>

class PackagingModel : public PackagingInterface {
public:
  PackagingModel();
  void simulatePackaging(std::shared_ptr<Wafer> wafer, double substrate_thickness,
                        const std::string& substrate_material, int num_wires) override;
  void performElectricalTest(std::shared_ptr<Wafer> wafer, const std::string& test_type) override;

private:
  void applyDieBonding(std::shared_ptr<Wafer> wafer, double substrate_thickness, const std::string& substrate_material);
  void applyWireBonding(std::shared_ptr<Wafer> wafer, int num_wires);
  double calculateResistance(const std::shared_ptr<Wafer>& wafer);
  double calculateCapacitance(const std::shared_ptr<Wafer>& wafer);
};