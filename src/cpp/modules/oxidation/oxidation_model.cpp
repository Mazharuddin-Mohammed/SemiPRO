#include "oxidation_model.hpp"
#include "../../core/utils.hpp"
#include <cmath>

OxidationModel::OxidationModel() {}

void OxidationModel::simulateOxidation(std::shared_ptr<Wafer> wafer, double temperature, double time) {
  // Enhanced oxidation with atmosphere support
  std::string atmosphere = "dry"; // Default atmosphere
  double oxide_thickness = calculateEnhancedOxideThickness(temperature, time, atmosphere);

  // Calculate oxidation stress
  double stress = calculateOxidationStress(oxide_thickness, temperature);

  wafer->applyLayer(oxide_thickness, "oxide");
  Logger::getInstance().log("Enhanced oxidation: thickness=" + std::to_string(oxide_thickness) +
                            "μm, temp=" + std::to_string(temperature) + "°C, time=" + std::to_string(time) +
                            "h, stress=" + std::to_string(stress) + "MPa");
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

double OxidationModel::calculateEnhancedOxideThickness(double temperature, double time, const std::string& atmosphere) const {
  // Enhanced Deal-Grove model with atmosphere dependence
  double temp_kelvin = temperature + 273.15;

  // Calculate atmosphere-dependent parameters
  double A, B0, Ea;

  if (atmosphere == "wet") {
    A = 0.090;    // μm (linear rate constant)
    B0 = 3.86e5;  // μm²/s (parabolic rate pre-factor)
    Ea = 2.05;    // eV (activation energy)
  } else { // dry oxidation
    A = 0.165;    // μm
    B0 = 1.23e6;  // μm²/s
    Ea = 2.0;     // eV
  }

  // Temperature-dependent parabolic rate constant
  double k_boltzmann = 8.617e-5; // eV/K
  double B = B0 * std::exp(-Ea / (k_boltzmann * temp_kelvin));

  // Initial oxide (native oxide)
  double X0 = 0.002; // 2 nm
  double tau = (X0 * X0 + A * X0) / B;

  // Convert time from hours to seconds
  double time_seconds = time * 3600.0;

  // Solve Deal-Grove equation: X² + AX = B(t + τ)
  double discriminant = A * A + 4 * B * (time_seconds + tau);
  double thickness = (-A + std::sqrt(discriminant)) / 2.0;

  return std::max(X0, thickness); // Ensure minimum native oxide
}

double OxidationModel::calculateOxidationStress(double thickness, double temperature) const {
  // Calculate stress due to volume expansion during oxidation

  // Material properties
  double youngs_modulus_sio2 = 70e9;   // Pa
  double poisson_ratio = 0.27;
  double volume_expansion_ratio = 2.27; // SiO2/Si volume ratio

  // Thermal expansion coefficients
  double alpha_si = 2.6e-6;    // /K
  double alpha_sio2 = 0.5e-6;  // /K

  // Volume expansion strain
  double volume_strain = (volume_expansion_ratio - 1.0) / 3.0;

  // Thermal strain (assuming room temperature reference)
  double temp_diff = temperature - 25.0;
  double thermal_strain = (alpha_sio2 - alpha_si) * temp_diff;

  // Total strain
  double total_strain = volume_strain + thermal_strain;

  // Biaxial stress (plane strain condition)
  double stress = (youngs_modulus_sio2 / (1 - poisson_ratio)) * total_strain;

  // Scale with thickness (thicker oxide = more stress)
  double thickness_factor = std::min(1.0, thickness / 0.1); // Normalize to 100 nm

  // Convert to MPa and apply thickness scaling
  return (stress / 1e6) * thickness_factor;
}