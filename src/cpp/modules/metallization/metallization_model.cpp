#include "metallization_model.hpp"
#include <stdexcept>

MetallizationModel::MetallizationModel() {}

void MetallizationModel::simulateMetallization(std::shared_ptr<Wafer> wafer, double thickness, const std::string& metal,
                                              const std::string& method) {
  if (thickness <= 0.0) {
    throw std::invalid_argument("Thickness must be positive");
  }
  if (metal.empty()) {
    throw std::invalid_argument("Metal type cannot be empty");
  }

  Logger::getInstance().log("Simulating metallization: thickness=" + std::to_string(thickness) + " um, metal=" + metal +
                  ", method=" + method);

  if (method == "pvd") {
    applyPVD(wafer, thickness, metal);
  } else if (method == "electroplating") {
    applyElectroplating(wafer, thickness, metal);
  } else {
    throw std::invalid_argument("Unknown metallization method: " + method);
  }

  wafer->addMetalLayer(thickness, metal);
}

void MetallizationModel::applyPVD(std::shared_ptr<Wafer> wafer, double thickness, const std::string& metal) {
  Eigen::ArrayXXd grid = wafer->getGrid();
  const Eigen::ArrayXXd& photoresist = wafer->getPhotoresistPattern();

  // PVD deposits uniformly in exposed areas (where photoresist is absent or patterned)
  for (int i = 0; i < grid.rows(); ++i) {
    for (int j = 0; j < grid.cols(); ++j) {
      if (i < photoresist.rows() && j < photoresist.cols() && photoresist(i, j) < 0.5) {
        grid(i, j) += thickness; // Deposit metal in exposed regions
      }
    }
  }

  wafer->updateGrid(grid);
}

void MetallizationModel::applyElectroplating(std::shared_ptr<Wafer> wafer, double thickness, const std::string& metal) {
  Eigen::ArrayXXd grid = wafer->getGrid();
  const Eigen::ArrayXXd& photoresist = wafer->getPhotoresistPattern();

  // Electroplating deposits selectively in patterned trenches (simplified model)
  for (int i = 0; i < grid.rows(); ++i) {
    for (int j = 0; j < grid.cols(); ++j) {
      if (i < photoresist.rows() && j < photoresist.cols() && photoresist(i, j) < 0.5) {
        grid(i, j) += thickness * 1.2; // Slightly thicker due to selective growth
      }
    }
  }

  wafer->updateGrid(grid);
}