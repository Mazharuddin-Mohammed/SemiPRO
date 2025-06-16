#include <catch2/catch_test_macros.hpp>
#include "../../src/cpp/modules/etching/etching_model.hpp"
#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/photolithography/lithography_model.hpp"

TEST_CASE("Isotropic etching simulation", "[Etching]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  LithographyModel lithography;
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  lithography.simulateExposure(wafer, 13.5, 0.33, mask);
  EtchingModel etching;
  double initial_sum = wafer->getGrid().sum();
  etching.simulateEtching(wafer, 0.05, "isotropic");
  REQUIRE(wafer->getGrid().sum() < initial_sum);
}

TEST_CASE("Anisotropic etching simulation", "[Etching]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  LithographyModel lithography;
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  lithography.simulateExposure(wafer, 13.5, 0.33, mask);
  EtchingModel etching;
  double initial_sum = wafer->getGrid().sum();
  etching.simulateEtching(wafer, 0.05, "anisotropic");
  REQUIRE(wafer->getGrid().sum() < initial_sum);
}