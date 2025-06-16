#include <catch2/catch_test_macros.hpp>
#include "../../src/cpp/modules/geometry/geometry_manager.hpp"

TEST_CASE("Geometry grid initialization", "[Geometry]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  GeometryManager geometry;
  geometry.initializeGrid(wafer, 10, 10);
  REQUIRE(wafer->getGrid().rows() == 10);
  REQUIRE(wafer->getGrid().cols() == 10);
}

TEST_CASE("Geometry apply layer", "[Geometry]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  GeometryManager geometry;
  geometry.applyLayer(wafer, 0.1, "oxide");
  REQUIRE(wafer->getThickness() == 775.1);
  REQUIRE(wafer->getMaterialId() == "oxide");
}