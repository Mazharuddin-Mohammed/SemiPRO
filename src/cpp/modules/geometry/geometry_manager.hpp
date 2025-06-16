#ifndef GEOMETRY_MANAGER_HPP
#define GEOMETRY_MANAGER_HPP

#include "geometry_interface.hpp"
#include <memory>

class GeometryManager : public GeometryInterface {
public:
  GeometryManager();
  void initializeGrid(std::shared_ptr<Wafer> wafer, int x_dim, int y_dim) override;
  void applyLayer(std::shared_ptr<Wafer> wafer, double thickness, const std::string& material_id) override;

private:
  std::shared_ptr<Wafer> wafer_;
};

#endif // GEOMETRY_MANAGER_HPP