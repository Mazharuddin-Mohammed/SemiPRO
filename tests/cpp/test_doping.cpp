#include <catch2/catch_test_macros.hpp>
#include "../../src/cpp/modules/doping/doping_manager.hpp"
#include "../../src/cpp/core/wafer.hpp"

TEST_CASE("Ion implantation", "[Doping]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(100, 100);
  DopingManager doping;
  doping.simulateIonImplantation(wafer, 50.0, 1e15);
  auto profile = wafer->getDopantProfile();
  REQUIRE(profile.sum() > 0.0); // Non-zero doping
}

TEST_CASE("Diffusion", "[Doping]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(100, 100);
  DopingManager doping;
  doping.simulateIonImplantation(wafer, 50.0, 1e15);
  auto initial_profile = wafer->getDopantProfile();
  doping.simulateDiffusion(wafer, 1000.0, 3600.0);
  auto final_profile = wafer->getDopantProfile();
  REQUIRE(final_profile.sum() > 0.0);
  REQUIRE(final_profile.maxCoeff() < initial_profile.maxCoeff()); // Diffusion spreads profile
}