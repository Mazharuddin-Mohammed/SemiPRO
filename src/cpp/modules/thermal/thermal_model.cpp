#include "thermal_model.hpp"
#include <stdexcept>
#include <Eigen/Sparse>

ThermalSimulationModel::ThermalSimulationModel() {}

void ThermalSimulationModel::simulateThermal(std::shared_ptr<Wafer> wafer, double ambient_temperature, double current) {
  if (ambient_temperature <= 0) {
    throw std::invalid_argument("Ambient temperature must be positive Kelvin");
  }
  if (current < 0) {
    throw std::invalid_argument("Current must be non-negative");
  }

  Logger::getInstance().log("Simulating thermal: ambient_temperature=" + std::to_string(ambient_temperature) +
                  " K, current=" + std::to_string(current) + " A");

  initializeThermalProperties(wafer);
  Eigen::ArrayXXd heat_source(wafer->getGrid().rows(), wafer->getGrid().cols());
  computeHeatSources(wafer, current, heat_source);
  solveHeatEquation(wafer, ambient_temperature, heat_source);
}

void ThermalSimulationModel::initializeThermalProperties(std::shared_ptr<Wafer> wafer) {
  Eigen::ArrayXXd conductivity = Eigen::ArrayXXd::Constant(wafer->getGrid().rows(), wafer->getGrid().cols(), 150.0); // Si
  const auto& film_layers = wafer->getFilmLayers();
  const auto& metal_layers = wafer->getMetalLayers();
  const auto& substrate = wafer->getPackagingSubstrate();
  const auto& photoresist = wafer->getPhotoresistPattern();

  for (int i = 0; i < conductivity.rows(); ++i) {
    for (int j = 0; j < conductivity.cols(); ++j) {
      if (i < photoresist.rows() && j < photoresist.cols() && photoresist(i, j) < 0.5 && !metal_layers.empty()) {
        conductivity(i, j) = 400.0; // Cu
      } else if (!film_layers.empty()) {
        conductivity(i, j) = 1.4; // SiO2
      }
      if (substrate.first > 0 && substrate.second == "Ceramic") {
        conductivity(i, j) = 20.0; // Ceramic
      }
    }
  }
  wafer->setThermalConductivity(conductivity);
}

void ThermalSimulationModel::computeHeatSources(std::shared_ptr<Wafer> wafer, double current, Eigen::ArrayXXd& heat_source) {
  heat_source.setZero();
  if (wafer->getMetalLayers().empty()) {
    return;
  }

  double resistance = 0.0;
  for (const auto& prop : wafer->getElectricalProperties()) {
    if (prop.first == "Resistance") {
      resistance = prop.second;
      break;
    }
  }
  if (resistance == 0.0) {
    Logger::getInstance().log("WARNING: No resistance data; assuming zero heat sources");
    return;
  }

  double power = current * current * resistance; // P = I^2 * R (W)
  double thickness = 0.0;
  for (const auto& layer : wafer->getMetalLayers()) {
    thickness += layer.first * 1e-6; // um to m
  }
  double area = 1e-6 * wafer->getGrid().rows() * 1e-6 * wafer->getGrid().cols(); // Grid area (m^2)
  double volume = area * thickness; // m^3
  double power_density = volume > 0 ? power / volume : 0.0; // W/m^3

  const auto& photoresist = wafer->getPhotoresistPattern();
  for (int i = 0; i < heat_source.rows(); ++i) {
    for (int j = 0; j < heat_source.cols(); ++j) {
      if (i < photoresist.rows() && j < photoresist.cols() && photoresist(i, j) < 0.5) {
        heat_source(i, j) = power_density;
      }
    }
  }
}

void ThermalSimulationModel::solveHeatEquation(std::shared_ptr<Wafer> wafer, double ambient_temperature,
                                              const Eigen::ArrayXXd& heat_source) {
  int rows = wafer->getGrid().rows();
  int cols = wafer->getGrid().cols();
  const auto& k = wafer->getThermalConductivity();
  int N = rows * cols;

  // Sparse matrix for FDM
  Eigen::SparseMatrix<double> A(N, N);
  Eigen::VectorXd b(N);
  std::vector<Eigen::Triplet<double>> triplets;

  double dx = 1e-6; // Grid spacing: 1 um
  double dx2 = dx * dx;

  // Build system
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      int idx = i * cols + j;
      if (i == 0 || i == rows - 1 || j == 0 || j == cols - 1) {
        triplets.emplace_back(idx, idx, 1.0);
        b(idx) = ambient_temperature;
      } else {
        double k_avg = k(i, j);
        triplets.emplace_back(idx, idx, 4.0);
        triplets.emplace_back(idx, (i-1)*cols + j, -1.0);
        triplets.emplace_back(idx, (i+1)*cols + j, -1.0);
        triplets.emplace_back(idx, i*cols + (j-1), -1.0);
        triplets.emplace_back(idx, i*cols + (j+1), -1.0);
        b(idx) = dx2 * heat_source(i, j) / k_avg;
      }
    }
  }

  A.setFromTriplets(triplets.begin(), triplets.end());
  Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
  solver.compute(A);
  if (solver.info() != Eigen::Success) {
    throw std::runtime_error("Thermal solver decomposition failed");
  }

  Eigen::VectorXd T_vec = solver.solve(b);
  if (solver.info() != Eigen::Success) {
    throw std::runtime_error("Thermal solver failed");
  }

  Eigen::ArrayXXd T(rows, cols);
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      T(i, j) = T_vec(i * cols + j);
    }
  }

  wafer->setTemperatureProfile(T);
  Logger::getInstance().log("Thermal simulation completed. Max temperature: " + std::to_string(T.maxCoeff()) + " K");
}