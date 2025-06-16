#pragma once
#include "metallization_interface.hpp"
#include "../../core/utils.hpp"
#include <Eigen/Dense>

class MetallizationModel : public MetallizationInterface {
public:
  MetallizationModel();
  void simulateMetallization(std::shared_ptr<Wafer> wafer, double thickness, const std::string& metal,
                            const std::string& method) override;

private:
  void applyPVD(std::shared_ptr<Wafer> wafer, double thickness, const std::string& metal);
  void applyElectroplating(std::shared_ptr<Wafer> wafer, double thickness, const std::string& metal);
};