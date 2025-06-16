#include "geometry_manager.hpp"
#include "../../core/utils.hpp"

GeometryManager::GeometryManager() {}

void GeometryManager::initializeGrid(std::shared_ptr<Wafer> wafer, int x_dim, int y_dim) {
  wafer->initializeGrid(x_dim, y_dim);
  Logger::getInstance().log("Grid initialized: " + std::to_string(x_dim) + "x" + std::to_string(y_dim));
}

void GeometryManager::applyLayer(std::shared_ptr<Wafer> wafer, double thickness, const std::string& material_id) {
  wafer->applyLayer(thickness, material_id);
  Logger::getInstance().log("Layer applied: thickness=" + std::to_string(thickness) + ", material=" + material_id);
}