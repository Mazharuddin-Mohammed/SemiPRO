#include "core/utils.hpp"
#include "core/simulation_engine.hpp"
#include "core/wafer_enhanced.hpp"
#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>

// JSON parsing (simple implementation)
#include <sstream>

std::unordered_map<std::string, std::string> parseSimpleJSON(const std::string& json_content) {
    std::unordered_map<std::string, std::string> result;

    // Very simple JSON parser for our specific format
    std::istringstream iss(json_content);
    std::string line;

    while (std::getline(iss, line)) {
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);

            // Remove quotes, spaces, commas
            key.erase(std::remove_if(key.begin(), key.end(), [](char c) {
                return c == '"' || c == ' ' || c == '\t';
            }), key.end());

            value.erase(std::remove_if(value.begin(), value.end(), [](char c) {
                return c == '"' || c == ' ' || c == '\t' || c == ',';
            }), value.end());

            result[key] = value;
        }
    }

    return result;
}

int main(int argc, char* argv[]) {
    try {
        // Basic debug output
        std::cout << "SemiPRO C++ Simulator starting..." << std::endl;

        // Initialize logging
        Logger::getInstance().log("SemiPRO C++ Simulator starting...");

        // Parse command line arguments
        std::string config_file;
        std::string process_type;

        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--config" && i + 1 < argc) {
                config_file = argv[i + 1];
                i++;
            } else if (arg == "--process" && i + 1 < argc) {
                process_type = argv[i + 1];
                i++;
            }
        }

        if (config_file.empty()) {
            std::cout << "No config file specified, running default simulation" << std::endl;
            Logger::getInstance().log("No config file specified, running default simulation");

            // Default simulation for testing
            auto& engine = SimulationEngine::getInstance();
            engine.initialize(""); // Initialize with empty config
            auto wafer = engine.createWafer(300.0, 775.0, "silicon");
            wafer->initializeGrid(50, 50);
            engine.registerWafer(wafer, "test_wafer");

            // Test oxidation
            SimulationEngine::ProcessParameters params("oxidation", 1.0);
            params.parameters["temperature"] = 1000.0;
            params.parameters["time"] = 1.0;
            params.parameters["ambient"] = 0.0; // dry

            auto future = engine.simulateProcessAsync("test_wafer", params);
            bool success = future.get();
            Logger::getInstance().log("Default oxidation test: " + std::string(success ? "SUCCESS" : "FAILED"));

            return success ? 0 : 1;
        }

        // Read configuration file
        std::ifstream file(config_file);
        if (!file.is_open()) {
            Logger::getInstance().log("Failed to open config file: " + config_file);
            return 1;
        }

        std::string json_content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
        file.close();

        auto config = parseSimpleJSON(json_content);

        // Create wafer and simulation engine
        auto& engine = SimulationEngine::getInstance();
        engine.initialize(config_file); // Initialize with config file
        auto wafer = engine.createWafer(300.0, 775.0, "silicon");
        wafer->initializeGrid(50, 50);
        engine.registerWafer(wafer, "main_wafer");

        // Handle different operation types
        bool success = false;

        if (process_type == "geometry_init") {
            // Geometry initialization - already done above
            success = true;
            Logger::getInstance().log("Geometry initialization completed");

        } else if (process_type == "grid_init") {
            // Grid initialization - already done above
            success = true;
            Logger::getInstance().log("Grid initialization completed");

        } else if (process_type == "oxidation") {
            SimulationEngine::ProcessParameters params(process_type, 1.0);
            params.parameters["temperature"] = std::stod(config.count("temperature") ? config["temperature"] : "1000");
            params.parameters["time"] = std::stod(config.count("time") ? config["time"] : "1.0");
            params.parameters["ambient"] = config.count("atmosphere") && config["atmosphere"] == "wet" ? 1.0 : 0.0;

            auto future = engine.simulateProcessAsync("main_wafer", params);
            success = future.get();

        } else if (process_type == "doping") {
            SimulationEngine::ProcessParameters params(process_type, 1.0);
            params.parameters["energy"] = std::stod(config.count("energy") ? config["energy"] : "50");
            params.parameters["dose"] = std::stod(config.count("concentration") ? config["concentration"] : "1e15");
            params.parameters["mass"] = 11.0; // Default: boron
            params.parameters["atomic_number"] = 5.0; // Default: boron

            auto future = engine.simulateProcessAsync("main_wafer", params);
            success = future.get();

        } else if (process_type == "deposition") {
            SimulationEngine::ProcessParameters params(process_type, 1.0);
            params.parameters["thickness"] = std::stod(config.count("thickness") ? config["thickness"] : "0.5");
            params.parameters["temperature"] = std::stod(config.count("temperature") ? config["temperature"] : "400");

            auto future = engine.simulateProcessAsync("main_wafer", params);
            success = future.get();

        } else if (process_type == "etching") {
            SimulationEngine::ProcessParameters params(process_type, 1.0);
            params.parameters["depth"] = std::stod(config.count("rate") ? config["rate"] : "0.5");
            params.parameters["type"] = config.count("etch_type") && config["etch_type"] == "isotropic" ? 0.0 : 1.0;
            params.parameters["selectivity"] = std::stod(config.count("selectivity") ? config["selectivity"] : "10");

            auto future = engine.simulateProcessAsync("main_wafer", params);
            success = future.get();

        } else {
            Logger::getInstance().log("Unknown process type: " + process_type);
            success = false;
        }

        Logger::getInstance().log("Simulation completed: " + process_type + " - " +
                                 std::string(success ? "SUCCESS" : "FAILED"));

        return success ? 0 : 1;

    } catch (const std::exception& e) {
        std::cerr << "Simulation failed with exception: " << e.what() << std::endl;
        Logger::getInstance().log("Simulation failed with exception: " + std::string(e.what()));
        return 1;
    }
}