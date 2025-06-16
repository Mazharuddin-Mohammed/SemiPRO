#ifndef GEOMETRY_INTERFACE_HPP
#define GEOMETRY_INTERFACE_HPP

#include "../../core/wafer.hpp"
#include <memory>

class GeometryInterface {
public:
  virtual ~GeometryInterface() = default;
  virtual void initializeGrid(std::shared_ptr<Wafer> wafer, int x_dim, int y_dim) = 0;
  virtual void applyLayer(std::shared_ptr<Wafer> wafer, double thickness, const std::string& material_id) = 0;
};

#endif // GEOMETRY_INTERFACE_HPP