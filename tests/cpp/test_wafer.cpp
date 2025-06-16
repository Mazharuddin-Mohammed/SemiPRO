#include <catch2/catch_test_macros.hpp>
#include "../../src/cpp/core/wafer.hpp"

TEST_CASE("Wafer initialization", "[Wafer]") {
  Wafer wafer(300.0, 775.0, "silicon");
  REQUIRE(wafer.getDiameter() == 300.0);
  REQUIRE(wafer.getThickness() == 775.0);
  REQUIRE(wafer.getMaterialId() == "silicon");
}

TEST_CASE("Wafer grid initialization", "[Wafer]") {
  Wafer wafer(300.0, 775.0, "silicon");
  wafer.initializeGrid(10, 10);
  REQUIRE(wafer.getGrid().rows() == 10);
  REQUIRE(wafer.getGrid().cols() == 10);
  REQUIRE(wafer.getGrid().sum() == 0.0);
}