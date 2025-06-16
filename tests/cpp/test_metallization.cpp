#include <catch2/catch_test_macros.hpp>
#include "../../src/cpp/modules/metallization/metallization_model.hpp"
#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/photolithography/lithography_model.hpp"

TEST_CASE("PVD metallization simulation", "[Metallization]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  // Apply photoresist
  LithographyModel lithography;
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  lithography.simulateExposure(wafer, 13.5, 0.33, mask);
  MetallizationModel metallization;
  double initial_sum = wafer->getGrid().sum();
  metallization.simulateMetallization(wafer, 0.5, "Cu", "pvd");
  REQUIRE(wafer->getGrid().sum() > initial_sum);
  REQUIRE(wafer->getMetalLayers().size() == 1);
  REQUIRE(wafer->getMetalLayers()[0].second == "Cu");
}

TEST_CASE("Electroplating simulation", "[Metallization]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  LithographyModel lithography;
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  lithography.simulateExposure(wafer, 13.5, 0.33, mask);
  double initial_sum = wafer->getGrid().sum();
  MetallizationModel metallization;
  metallization.simulateMetallization(wafer, 0.5, "Cu", "electroplating");
  REQUIRE(wafer->getGrid().sum() > initial_sum);
}