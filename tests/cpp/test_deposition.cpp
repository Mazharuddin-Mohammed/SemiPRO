#include <catch2/catch_test_macros.hpp>
#include "../../src/cpp/modules/deposition/deposition_model.hpp"
#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/photolithography/lithography_model.hpp"

TEST_CASE("Uniform deposition simulation", "[Deposition]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  // Apply photoresist pattern
  LithographyModel lithography;
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  lithography.simulateExposure(wafer, 13.5, 0.33, mask);
  DepositionModel deposition;
  double initial_sum = wafer->getGrid().sum();
  deposition.simulateDeposition(wafer, 0.1, "SiO2", "uniform");
  REQUIRE(wafer->getGrid().sum() > initial_sum);
  REQUIRE(wafer->getFilmLayers().size() == 1);
  REQUIRE(wafer->getFilmLayers()[0].second == "SiO2");
}

TEST_CASE("Conformal deposition simulation", "[Deposition]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  LithographyModel lithography;
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  lithography.simulateExposure(wafer, 13.5, 0.33, mask);
  DepositionModel deposition;
  double initial_sum = wafer->getGrid().sum();
  deposition.simulateDeposition(wafer, 0.1, "SiO2", "conformal");
  REQUIRE(wafer->getGrid().sum() > initial_sum);
}