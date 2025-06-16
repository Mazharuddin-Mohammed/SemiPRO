#include <catch2/catch_test_macros.hpp>
#include "../../src/cpp/modules/oxidation/oxidation_model.hpp"
#include "../../src/cpp/core/wafer.hpp"

TEST_CASE("Oxidation thickness calculation", "[Oxidation]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  OxidationModel oxidation;
  oxidation.simulateOxidation(wafer, 1000.0, 3600.0);
  REQUIRE(wafer->getMaterialId() == "oxide");
  REQUIRE(wafer->getThickness() > 775.0); // Oxide layer added
}