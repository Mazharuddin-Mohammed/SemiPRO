#include <catch2/catch_test_macros.hpp>
#include "../../src/cpp/modules/photolithography/lithography_model.hpp"
#include "../../src/cpp/core/wafer.hpp"

TEST_CASE("Exposure simulation", "[Photolithography]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  LithographyModel lithography;
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  lithography.simulateExposure(wafer, 13.5, 0.33, mask);
  auto pattern = wafer->getPhotoresistPattern();
  REQUIRE(pattern.sum() > 0.0); // Pattern exists
}

TEST_CASE("Multi-patterning simulation", "[Photolithography]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  LithographyModel lithography;
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  std::vector<std::vector<std::vector<int>>> masks = {mask, mask};
  lithography.simulateMultiPatterning(wafer, 13.5, 0.33, masks);
  auto pattern = wafer->getPhotoresistPattern();
  REQUIRE(pattern.sum() > 0.0);
}

TEST_CASE("Exposure simulation with Gaussian PSF", "[Photolithography]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  LithographyModel lithography;
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  lithography.simulateExposure(wafer, 13.5, 0.33, mask);
  auto pattern = wafer->getPhotoresistPattern();
  REQUIRE(pattern.sum() > 0.0);
  REQUIRE((pattern >= 0.0).all()); // Pattern values >= 0
  REQUIRE((pattern <= 1.0).all()); // Pattern values <= 1
}