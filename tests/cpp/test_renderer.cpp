#include <catch2/catch_test_macros.hpp>
#include "../../src/cpp/renderer/vulkan_renderer.hpp"
#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/doping/doping_manager.hpp"
#include "../../src/cpp/modules/photolithography/lithography_model.hpp"
#include "../../src/cpp/modules/deposition/deposition_model.hpp"
#include "../../src/cpp/modules/etching/etching_model.hpp"
#include "../../src/cpp/modules/metallization/metallization_model.hpp"
#include "../../src/cpp/modules/packaging/packaging_model.hpp"
#include "../../src/cpp/modules/thermal/thermal_model.hpp"

TEST_CASE("Renderer initialization", "[Renderer]") {
  VulkanRenderer renderer(400, 400);
  REQUIRE_NOTHROW(renderer.initialize());
}

TEST_CASE("Renderer with oxide layer", "[Renderer]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  wafer->applyLayer(0.1, "oxide");
  VulkanRenderer renderer(400, 400);
  renderer.initialize();
  REQUIRE_NOTHROW(renderer.render(wafer));
}

TEST_CASE("Renderer with dopant profile", "[Renderer]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  DopingManager doping;
  doping.simulateIonImplantation(wafer, 50.0, 1e15);
  VulkanRenderer renderer(400, 400);
  renderer.initialize();
  REQUIRE_NOTHROW(renderer.render(wafer));
}

TEST_CASE("Renderer with photoresist pattern", "[Renderer]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  LithographyModel lithography;
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  lithography.simulateExposure(wafer, 13.5, 0.33, mask);
  VulkanRenderer renderer(400, 400);
  renderer.initialize();
  REQUIRE_NOTHROW(renderer.render(wafer));
}

TEST_CASE("Renderer with deposited film", "[Renderer]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  LithographyModel lithography;
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  lithography.simulateExposure(wafer, 13.5, 0.33, mask);
  DepositionModel deposition;
  deposition.simulateDeposition(wafer, 0.1, "SiO2", "uniform");
  VulkanRenderer renderer(400, 400);
  renderer.initialize();
  REQUIRE_NOTHROW(renderer.render(wafer));
}

TEST_CASE("Renderer with etched region", "[Renderer]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  LithographyModel lithography;
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  lithography.simulateExposure(wafer, 13.5, 0.33, mask);
  EtchingModel etching;
  etching.simulateEtching(wafer, 0.05, "anisotropic");
  VulkanRenderer renderer(400, 400);
  renderer.initialize();
  REQUIRE_NOTHROW(renderer.render(wafer));
}

TEST_CASE("Renderer with metal layer", "[Renderer]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  LithographyModel lithography;
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  lithography.simulateExposure(wafer, 13.5, 0.33, mask);
  MetallizationModel metallization;
  metallization.simulateMetallization(wafer, 0.5, "Cu", "pvd");
  VulkanRenderer renderer(400, 400);
  renderer.initialize();
  REQUIRE_NOTHROW(renderer.render(wafer));
}

TEST_CASE("Renderer with packaging and electrical test", "[Renderer]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  LithographyModel lithography;
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  lithography.simulateExposure(wafer, 13.5, 0.33, mask);
  MetallizationModel metallization;
  metallization.simulateMetallization(wafer, 0.5, "Cu", "pvd");
  PackagingModel packaging;
  packaging.simulatePackaging(wafer, 1000.0, "Ceramic", 2);
  packaging.performElectricalTest(wafer, "resistance");
  VulkanRenderer renderer(400, 400);
  renderer.initialize();
  REQUIRE_NOTHROW(renderer.render(wafer));
}

TEST_CASE("Renderer with thermal simulation", "[Renderer]") {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  wafer->initializeGrid(10, 10);
  LithographyModel lithography;
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  lithography.simulateExposure(wafer, 13.5, 0.33, mask);
  MetallizationModel metallization;
  metallization.simulateMetallization(wafer, 0.5, "Cu", "pvd");
  PackagingModel packaging;
  packaging.performElectricalTest(wafer, "resistance");
  ThermalSimulationModel thermal;
  thermal.simulateThermal(wafer, 300.0, 0.001);
  VulkanRenderer renderer(400, 400);
  renderer.initialize();
  REQUIRE_NOTHROW(renderer.render(wafer));
}