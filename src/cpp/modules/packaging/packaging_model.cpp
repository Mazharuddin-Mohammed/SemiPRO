#include "packaging_model.hpp"
#include <stdexcept>
#include <random>

PackagingModel::PackagingModel() {}

void PackagingModel::simulatePackaging(std::shared_ptr<Wafer> wafer, double substrate_thickness,
                                      const std::string& substrate_material, int num_wires) {
  if (substrate_thickness <= 0.0) {
    throw std::invalid_argument("Substrate thickness must be positive");
  }
  if (num_wires < 0) {
    throw std::invalid_argument("Number of wires cannot be negative");
  }

  Logger::getInstance().log("Simulating packaging: substrate_thickness=" + std::to_string(substrate_thickness) +
                  " um, material=" + substrate_material + ", wires=" + std::to_string(num_wires));

  applyDieBonding(wafer, substrate_thickness, substrate_material);
  applyWireBonding(wafer, num_wires);
}

void PackagingModel::applyDieBonding(std::shared_ptr<Wafer> wafer, double substrate_thickness,
                                    const std::string& substrate_material) {
  std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> empty_wires;
  wafer->addPackaging(substrate_thickness, substrate_material, empty_wires);
}

void PackagingModel::applyWireBonding(std::shared_ptr<Wafer> wafer, int num_wires) {
  const Eigen::ArrayXXd& photoresist = wafer->getPhotoresistPattern();
  if (wafer->getMetalLayers().empty()) {
    throw std::runtime_error("No metal layers for wire bonding");
  }
  std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> wire_bonds;

  std::vector<std::pair<int, int>> pads;
  for (int i = 0; i < photoresist.rows(); ++i) {
    for (int j = 0; j < photoresist.cols(); ++j) {
      if (i < photoresist.rows() && j < photoresist.cols() && photoresist(i, j) < 0.5) {
        pads.emplace_back(i, j);
      }
    }
  }

  if (pads.size() < 2) {
    throw std::runtime_error("Insufficient pads for wire bonding");
  }

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, pads.size() - 1);
  int wires_to_add = std::min(num_wires, static_cast<int>(pads.size() / 2));

  for (int i = 0; i < wires_to_add; ++i) {
    int idx1 = dis(gen);
    int idx2 = dis(gen);
    while (idx2 == idx1 && pads.size() > 1) {
      idx2 = dis(gen);
    }
    wire_bonds.emplace_back(pads[idx1], pads[idx2]);
  }

  auto substrate = wafer->getPackagingSubstrate();
  wafer->addPackaging(substrate.first, substrate.second, wire_bonds);
}

void PackagingModel::performElectricalTest(std::shared_ptr<Wafer> wafer, const std::string& test_type) {
  if (wafer->getMetalLayers().empty()) {
    throw std::runtime_error("No metal layers for electrical testing");
  }
  std::vector<std::pair<std::string, double>> properties;
  if (test_type == "resistance") {
    double resistance = calculateResistance(wafer);
    properties.emplace_back("Resistance", resistance);
    Logger::getInstance().log("Electrical test: Resistance = " + std::to_string(resistance) + " Ohms");
  } else if (test_type == "capacitance") {
    double capacitance = calculateCapacitance(wafer);
    properties.emplace_back("Capacitance", capacitance);
    Logger::getInstance().log("Electrical test: Capacitance = " + std::to_string(capacitance) + " pF");
  } else {
    throw std::invalid_argument("Unknown test type: " + test_type);
  }
  wafer->setElectricalProperties(properties);
}

double PackagingModel::calculateResistance(const std::shared_ptr<Wafer>& wafer) {
  double resistivity = 1.68e-8; // Cu resistivity (Ω·m)
  double length = 1e-3; // 1 mm
  double thickness = 0.0;
  for (const auto& layer : wafer->getMetalLayers()) {
    thickness += layer.first * 1e-6; // um to m
  }
  double width = 1e-6; // 1 um
  double area = thickness * width;
  if (area == 0) return 0.0;
  return resistivity * length / area; // Ohms
}

double PackagingModel::calculateCapacitance(const std::shared_ptr<Wafer>& wafer) {
  double epsilon = 8.854e-12; // Vacuum permittivity (F/m)
  double length = 1e-3; // 1 mm
  double thickness = 0.0;
  for (const auto& layer : wafer->getMetalLayers()) {
    thickness += layer.first * 1e-6; // um to m
  }
  double area = length * thickness;
  double distance = 1e-6; // 1 um between lines
  if (distance == 0 || thickness == 0) return 0.0;
  return (epsilon * area / distance) * 1e12; // pF
}