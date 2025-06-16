#include "oxidation_model.hpp"
#include "../../core/utils.hpp"
#include <cmath>

OxidationModel::OxidationModel() {}

void OxidationModel::simulateOxidation(std::shared_ptr<Wafer> wafer, double temperature, double time) {
  double oxide_thickness = calculateOxideThickness(temperature, time);
  wafer->applyLayer(oxide_thickness, "oxide");
  Logger::getInstance().log("Oxidation applied: thickness=" + std::to_string(oxide_thickness) +
                            ", temp=" + std::to_string(temperature) + "K, time=" + std::to_string(time) + "s");
}

double OxidationModel::calculateOxideThickness(double temperature, double time) const {
  // Deal-Grove model parameters (simplified for dry oxidation)
  const double A = 0.165; // um
  const double B = 0.0117 * std::exp(-2.0 / (8.617e-5 * temperature)); // um^2/s
  double tau = 0.0; // Initial oxide thickness = 0

  // Solve quadratic equation: x^2 + A*x - B*(t + tau) = 0
  double discriminant = A * A + 4 * B * (time + tau);
  return (-A + std::sqrt(discriminant)) / 2.0;
}