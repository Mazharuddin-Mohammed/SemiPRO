#include "core/utils.hpp"
#include "modules/geometry/geometry_manager.hpp"
#include <memory>

int main() {
  Logger::getInstance().initialize("config/logging.yaml");
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  GeometryManager geometry;
  geometry.initializeGrid(wafer, 100, 100);
  geometry.applyLayer(wafer, 0.1, "silicon");
  return 0;
}