#include "deposition_model.hpp"
#include "../../core/utils.hpp"
#include <cmath>

DepositionModel::DepositionModel() {}

void DepositionModel::simulateDeposition(std::shared_ptr<Wafer> wafer, double thickness, const std::string& material, const std::string& type) {
    if (type == "uniform") {
        simulate_uniform(wafer, thickness, material);
    } else if (type == "conformal") {
        simulate_conformal(wafer, thickness, material);
    } else {
        Logger::getInstance().log("ERROR: Unknown deposition type: " + type);
        return;
    }
    wafer->addFilmLayer(thickness, material);
    Logger::getInstance().log("Deposition simulated: type=" + type + ", thickness=" + std::to_string(thickness) + "um, material=" + material);
}

void DepositionModel::simulate_uniform(std::shared_ptr<Wafer> wafer, double thickness, const std::string& material) {
    Eigen::ArrayXXd grid = wafer->getGrid();
    const Eigen::ArrayXXd& pattern = wafer->getPhotoresistPattern();
    for (int i = 0; i < grid.rows(); ++i) {
        for (int j = 0; j < grid.cols(); ++j) {
            if (i < pattern.rows() && j < pattern.cols() && pattern(i, j) < 0.5) { // No photoresist
                grid(i, j) += thickness;
            }
        }
    }
    wafer->updateGrid(grid);
}

void DepositionModel::simulate_conformal(std::shared_ptr<Wafer> wafer, double thickness, const std::string& material) {
    Eigen::ArrayXXd grid = wafer->getGrid();
    const Eigen::ArrayXXd& pattern = wafer->getPhotoresistPattern();
    Eigen::ArrayXXd new_grid = grid;
    for (int i = 1; i < grid.rows() - 1; ++i) {
        for (int j = 1; j < grid.cols() - 1; ++j) {
            if (i < pattern.rows() && j < pattern.cols() && pattern(i, j) < 0.5) {
                double laplacian = (grid(i+1,j) + grid(i-1,j) + grid(i,j+1) + grid(i,j-1) - 4 * grid(i,j)) / 4.0;
                new_grid(i,j) += thickness * (1.0 + 0.5 * std::abs(laplacian));
            }
        }
    }
    wafer->updateGrid(new_grid);
}