// Author: Dr. Mazharuddin Mohammed
#pragma once
#include <Eigen/Dense>
#include <vector>
#include <string>
#include <memory>

class Wafer {
public:
  Wafer(double diameter, double thickness, const std::string& material_id);
  void initializeGrid(int x_dim, int y_dim);
  void applyLayer(double thickness, const std::string& material_id);
  void setDopantProfile(const Eigen::ArrayXd& profile);
  void setPhotoresistPattern(const Eigen::ArrayXXd& pattern);
  void addFilmLayer(double thickness, const std::string& material);
  void addMetalLayer(double thickness, const std::string& metal);
  void addPackaging(double substrate_thickness, const std::string& substrate_material,
                    const std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>>& wire_bonds);
  void setElectricalProperties(const std::vector<std::pair<std::string, double>>& properties);
  void setTemperatureProfile(const Eigen::ArrayXXd& temperature);
  void setThermalConductivity(const Eigen::ArrayXXd& conductivity);
  void setElectromigrationMTTF(const Eigen::ArrayXXd& mttf); // NEW
  void setThermalStress(const Eigen::ArrayXXd& stress); // NEW
  void setDielectricField(const Eigen::ArrayXXd& field); // NEW
  void updateGrid(const Eigen::ArrayXXd& new_grid);
  Eigen::ArrayXXd& getGrid();
  const Eigen::ArrayXXd& getGrid() const;
  Eigen::ArrayXd& getDopantProfile();
  const Eigen::ArrayXd& getDopantProfile() const;
  Eigen::ArrayXXd& getPhotoresistPattern();
  const Eigen::ArrayXXd& getPhotoresistPattern() const;
  std::vector<std::pair<double, std::string>>& getFilmLayers();
  const std::vector<std::pair<double, std::string>>& getFilmLayers() const;
  std::vector<std::pair<double, std::string>>& getMetalLayers();
  const std::vector<std::pair<double, std::string>>& getMetalLayers() const;
  std::pair<double, std::string> getPackagingSubstrate() const;
  const std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>>& getWireBonds() const;
  const std::vector<std::pair<std::string, double>>& getElectricalProperties() const;
  const Eigen::ArrayXXd& getTemperatureProfile() const;
  const Eigen::ArrayXXd& getThermalConductivity() const;
  const Eigen::ArrayXXd& getElectromigrationMTTF() const; // NEW
  const Eigen::ArrayXXd& getThermalStress() const; // NEW
  const Eigen::ArrayXXd& getDielectricField() const; // NEW
  std::string getMaterialId() const;
  double getDiameter() const;
  double getThickness() const;

protected:
  double diameter_;
  double thickness_;
  std::string material_id_;
  Eigen::ArrayXXd grid_;
  Eigen::ArrayXd dopant_profile_;
  Eigen::ArrayXXd photoresist_pattern_;
  std::vector<std::pair<double, std::string>> film_layers_;
  std::vector<std::pair<double, std::string>> metal_layers_;
  std::pair<double, std::string> packaging_substrate_;
  std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> wire_bonds_;
  std::vector<std::pair<std::string, double>> electrical_properties_;
  Eigen::ArrayXXd temperature_profile_;
  Eigen::ArrayXXd thermal_conductivity_;
  Eigen::ArrayXXd electromigration_mttf_; // NEW
  Eigen::ArrayXXd thermal_stress_; // NEW
  Eigen::ArrayXXd dielectric_field_; // NEW
};