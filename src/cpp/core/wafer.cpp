#include "wafer.hpp"

Wafer::Wafer(double diameter, double thickness, const std::string& material_id)
    : diameter_(diameter), thickness_(thickness), material_id_(material_id), packaging_substrate_{0.0, ""} {}

void Wafer::initializeGrid(int x_dim, int y_dim) {
  grid_ = Eigen::ArrayXXd::Ones(x_dim, y_dim) * thickness_;
  dopant_profile_.resize(x_dim);
  dopant_profile_.setZero();
  photoresist_pattern_ = Eigen::ArrayXXd::Zero(x_dim, y_dim);
  temperature_profile_ = Eigen::ArrayXXd::Constant(x_dim, y_dim, 300.0); // Default 300 K
  thermal_conductivity_ = Eigen::ArrayXXd::Constant(x_dim, y_dim, 150.0); // Default Si conductivity
  electromigration_mttf_ = Eigen::ArrayXXd::Zero(x_dim, y_dim); // NEW
  thermal_stress_ = Eigen::ArrayXXd::Zero(x_dim, y_dim); // NEW
  dielectric_field_ = Eigen::ArrayXXd::Zero(x_dim, y_dim); // NEW
}

void Wafer::applyLayer(double thickness, const std::string& material_id) {
  grid_ += thickness;
  film_layers_.emplace_back(thickness, material_id);
}

void Wafer::setDopantProfile(const Eigen::ArrayXd& profile) {
  dopant_profile_ = profile;
}

void Wafer::setPhotoresistPattern(const Eigen::ArrayXXd& pattern) {
  photoresist_pattern_ = pattern;
}

void Wafer::addFilmLayer(double thickness, const std::string& material) {
  film_layers_.emplace_back(thickness, material);
}

void Wafer::addMetalLayer(double thickness, const std::string& metal) {
  metal_layers_.emplace_back(thickness, metal);
}

void Wafer::addPackaging(double substrate_thickness, const std::string& substrate_material,
                         const std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>>& wire_bonds) {
  packaging_substrate_ = {substrate_thickness, substrate_material};
  wire_bonds_ = wire_bonds;
}

void Wafer::setElectricalProperties(const std::vector<std::pair<std::string, double>>& properties) {
  electrical_properties_ = properties;
}

void Wafer::setTemperatureProfile(const Eigen::ArrayXXd& temperature) {
  temperature_profile_ = temperature;
}

void Wafer::setThermalConductivity(const Eigen::ArrayXXd& conductivity) {
  thermal_conductivity_ = conductivity;
}

void Wafer::setElectromigrationMTTF(const Eigen::ArrayXXd& mttf) { // NEW
  electromigration_mttf_ = mttf;
}

void Wafer::setThermalStress(const Eigen::ArrayXXd& stress) { // NEW
  thermal_stress_ = stress;
}

void Wafer::setDielectricField(const Eigen::ArrayXXd& field) { // NEW
  dielectric_field_ = field;
}

void Wafer::updateGrid(const Eigen::ArrayXXd& new_grid) {
  grid_ = new_grid;
}

Eigen::ArrayXXd& Wafer::getGrid() { return grid_; }
const Eigen::ArrayXXd& Wafer::getGrid() const { return grid_; }
Eigen::ArrayXd& Wafer::getDopantProfile() { return dopant_profile_; }
const Eigen::ArrayXd& Wafer::getDopantProfile() const { return dopant_profile_; }
Eigen::ArrayXXd& Wafer::getPhotoresistPattern() { return photoresist_pattern_; }
const Eigen::ArrayXXd& Wafer::getPhotoresistPattern() const { return photoresist_pattern_; }
std::vector<std::pair<double, std::string>>& Wafer::getFilmLayers() { return film_layers_; }
const std::vector<std::pair<double, std::string>>& Wafer::getFilmLayers() const { return film_layers_; }
std::vector<std::pair<double, std::string>>& Wafer::getMetalLayers() { return metal_layers_; }
const std::vector<std::pair<double, std::string>>& Wafer::getMetalLayers() const { return metal_layers_; }
std::pair<double, std::string> Wafer::getPackagingSubstrate() const { return packaging_substrate_; }
const std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>>& Wafer::getWireBonds() const { return wire_bonds_; }
const std::vector<std::pair<std::string, double>>& Wafer::getElectricalProperties() const { return electrical_properties_; }
const Eigen::ArrayXXd& Wafer::getTemperatureProfile() const { return temperature_profile_; }
const Eigen::ArrayXXd& Wafer::getThermalConductivity() const { return thermal_conductivity_; }
const Eigen::ArrayXXd& Wafer::getElectromigrationMTTF() const { return electromigration_mttf_; } // NEW
const Eigen::ArrayXXd& Wafer::getThermalStress() const { return thermal_stress_; } // NEW
const Eigen::ArrayXXd& Wafer::getDielectricField() const { return dielectric_field_; } // NEW
std::string Wafer::getMaterialId() const { return material_id_; }
double Wafer::getDiameter() const { return diameter_; }
double Wafer::getThickness() const { return thickness_; }