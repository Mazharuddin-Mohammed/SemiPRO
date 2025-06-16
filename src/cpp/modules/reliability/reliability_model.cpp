#include "reliability_model.hpp"
#include <stdexcept>
#include <cmath>

ReliabilityModel::ReliabilityModel() {}

void ReliabilityModel::performReliabilityTest(std::shared_ptr<Wafer> wafer, double current, double voltage) {
    if (current < 0) {
        throw std::invalid_argument("Current must be non-negative");
    }
    if (voltage < 0) {
        throw std::invalid_argument("Voltage must be non-negative");
    }

    Logger::getInstance().log("Performing reliability test: current=" + std::to_string(current) +
                    " A, voltage=" + std::to_string(voltage) + " V");

    computeElectromigration(wafer, current);
    computeThermalStress(wafer);
    computeDielectricBreakdown(wafer, voltage);
}

void ReliabilityModel::computeElectromigration(std::shared_ptr<Wafer> wafer, double current) {
    Eigen::ArrayXXd mttf = Eigen::ArrayXXd::Zero(wafer->getGrid().rows(), wafer->getGrid().cols());
    if (wafer->getMetalLayers().empty()) {
        wafer->setElectromigrationMTTF(mttf);
        Logger::getInstance().log("WARNING: No metal layers; electromigration MTTF set to zero");
        return;
    }

    const auto& photoresist = wafer->getPhotoresistPattern();
    const auto& temperature = wafer->getTemperatureProfile();
    double thickness = 0.0;
    for (const auto& layer : wafer->getMetalLayers()) {
        thickness += layer.first * 1e-6; // um to m
    }
    double dx = 1e-6; // Grid spacing: 1 um
    double area = dx * thickness * 10000; // m² to cm²
    double J = current / area; // Current density (A/cm²)

    const double A = 1e-10; // Scaling factor (s·A²/cm⁴)
    const double n = 2.0; // Current exponent
    const double Ea = 0.9; // Activation energy (eV)
    const double kB = 8.617e-5; // Boltzmann constant (eV/K)

    for (int i = 0; i < mttf.rows(); ++i) {
        for (int j = 0; j < mttf.cols(); ++j) {
            if (i < photoresist.rows() && j < photoresist.cols() && photoresist(i, j) < 0.5) {
                double T = temperature(i, j);
                if (T > 0) {
                    mttf(i, j) = A * pow(J, -n) * exp(Ea / (kB * T)); // Black’s equation
                }
            }
        }
    }

    wafer->setElectromigrationMTTF(mttf);
    Logger::getInstance().log("Electromigration computed. Min MTTF: " + std::to_string(mttf.minCoeff()) + " s");
}

void ReliabilityModel::computeThermalStress(std::shared_ptr<Wafer> wafer) {
    Eigen::ArrayXXd stress = Eigen::ArrayXXd::Zero(wafer->getGrid().rows(), wafer->getGrid().cols());
    const auto& temperature = wafer->getTemperatureProfile();
    const auto& film_layers = wafer->getFilmLayers();
    const auto& metal_layers = wafer->getMetalLayers();
    const auto& substrate = wafer->getPackagingSubstrate();
    const auto& photoresist = wafer->getPhotoresistPattern();
    double ambient_temp = 300.0; // K
    double dx = 1e-6; // Grid spacing: 1 um

    for (int i = 0; i < stress.rows(); ++i) {
        for (int j = 0; j < stress.cols(); ++j) {
            double E = 130e3; // Young’s modulus (MPa), Si
            double alpha = 2.6e-6; // Thermal expansion coefficient (/K), Si
            if (i < photoresist.rows() && j < photoresist.cols() && photoresist(i, j) < 0.5 && !metal_layers.empty()) {
                E = 110e3; // Cu
                alpha = 17e-6;
            } else if (!film_layers.empty()) {
                E = 70e3; // SiO2
                alpha = 0.5e-6;
            } else if (substrate.first > 0 && substrate.second == "Ceramic") {
                E = 300e3; // Ceramic
                alpha = 7e-6;
            }
            double delta_T = temperature(i, j) - ambient_temp;
            double base_stress = E * alpha * delta_T; // MPa

            // Add stress gradient contribution
            double gradient_stress = 0.0;
            if (i > 0 && i < stress.rows() - 1 && j > 0 && j < stress.cols() - 1) {
                double dT_dx = (temperature(i+1, j) - temperature(i-1, j)) / (2 * dx);
                double dT_dy = (temperature(i, j+1) - temperature(i, j-1)) / (2 * dx);
                double gradient = sqrt(dT_dx * dT_dx + dT_dy * dT_dy);
                gradient_stress = E * alpha * gradient * dx; // Simplified gradient effect
            }
            stress(i, j) = base_stress + gradient_stress;
        }
    }

    wafer->setThermalStress(stress);
    Logger::getInstance().log("Thermal stress computed. Max stress: " + std::to_string(stress.maxCoeff()) + " MPa");
}

void ReliabilityModel::computeDielectricBreakdown(std::shared_ptr<Wafer> wafer, double voltage) {
    Eigen::ArrayXXd field = Eigen::ArrayXXd::Zero(wafer->getGrid().rows(), wafer->getGrid().cols());
    const auto& film_layers = wafer->getFilmLayers();
    if (film_layers.empty()) {
        wafer->setDielectricField(field);
        Logger::getInstance().log("WARNING: No dielectric layers; electric field set to zero");
        return;
    }

    double thickness = 0.0;
    for (const auto& layer : film_layers) {
        if (layer.second == "SiO2") {
            thickness += layer.first * 1e-4; // um to cm
        }
    }
    if (thickness == 0.0) {
        wafer->setDielectricField(field);
        Logger::getInstance().log("WARNING: No SiO2 layers; electric field set to zero");
        return;
    }

    double E = voltage / thickness; // V/cm
    for (int i = 0; i < field.rows(); ++i) {
        for (int j = 0; j < field.cols(); ++j) {
            if (!film_layers.empty()) {
                field(i, j) = E;
            }
        }
    }

    wafer->setDielectricField(field);
    Logger::getInstance().log("Dielectric field computed. Field: " + std::to_string(E) + " V/cm");
}