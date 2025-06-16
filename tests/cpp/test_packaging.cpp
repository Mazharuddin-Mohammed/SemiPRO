#include <catch2/catch_test_macros.hpp>
#include "../../src/cpp/modules/packaging/packaging_model.hpp"
#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/photolithography/lithography_model.hpp"
#include "../../src/cpp/modules/metallization/metallization_model.hpp"

TEST_CASE("Packaging simulation", "[Packaging]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  // Apply metal and pattern
  LithographyModel lithography;
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  lithography.simulateExposure(wafer, 13.5, 0.33, mask);
  MetallizationModel metallization;
  metallization.simulateMetallization(wafer, 0.5, "Cu", "pvd");
  PackagingModel packaging;
  packaging.simulatePackaging(wafer, 1000.0, "Ceramic", 2);
  REQUIRE(wafer->getPackagingSubstrate().first == 1000.0);
  REQUIRE(wafer->getPackagingSubstrate().second == "Ceramic");
  REQUIRE(wafer->getWireBonds().size() <= 2);
}

TEST_CASE("Electrical resistance test", "[Packaging]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  LithographyModel lithography;
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  lithography.simulateExposure(wafer, 13.5, 0.33, mask);
  MetallizationModel metallization;
  metallization.simulateMetallization(wafer, 0.5, "Cu", "pvd");
  PackagingModel packaging;
  packaging.performElectricalTest(wafer, "resistance");
  auto properties = wafer->getElectricalProperties();
  REQUIRE(!properties.empty());
  REQUIRE(properties[0].first == "Resistance");
  REQUIRE(properties[0].second > 0.0);
}

TEST_CASE("Electrical capacitance test", "[Packaging]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  LithographyModel lithography;
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  lithography.simulateExposure(wafer, 13.5, 0.33, mask);
  MetallizationModel metallization;
  metallization.simulateMetallization(wafer, 0.5, "Cu", "pvd");
  PackagingModel packaging;
  packaging.performElectricalTest(wafer, "capacitance");
  auto properties = wafer->getElectricalProperties();
  REQUIRE(!properties.empty());
  REQUIRE(properties[0].first == "Capacitance");
  REQUIRE(properties[0].second > 0.0);
}