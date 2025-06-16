#include <catch2/catch_test_macros.hpp>
#include "../../src/cpp/modules/thermal/thermal_model.hpp"
#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/photolithography/lithography_model.hpp"
#include "../../src/cpp/modules/metallization/metallization_model.hpp"
#include "../../src/cpp/modules/packaging/packaging_model.hpp"

TEST_CASE("Thermal simulation", "[Thermal]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  LithographyModel lithography;
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  lithography.simulateExposure(wafer, 13.5, 0.33, mask);
  MetallizationModel metallization;
  metallization.simulateMetallization(wafer, 0.5, "Cu", "pvd");
  PackagingModel packaging;
  packaging.performElectricalTest(wafer, "resistance");
  packaging.simulatePackaging(wafer, 1000.0, "Ceramic", 2);
  ThermalSimulationModel thermal;
  thermal.simulateThermal(wafer, 300.0, 0.001);
  auto temp_profile = wafer->getTemperatureProfile();
  REQUIRE(temp_profile.rows() == 10);
  REQUIRE(temp_profile.cols() == 10);
  REQUIRE(temp_profile.minCoeff() >= 300.0);
}

TEST_CASE("Thermal simulation no metal", "[Thermal]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  ThermalSimulationModel thermal;
  thermal.simulateThermal(wafer, 300.0, 0.001);
  auto temp_profile = wafer->getTemperatureProfile();
  REQUIRE((temp_profile == 300.0).all()); // No heat sources
}