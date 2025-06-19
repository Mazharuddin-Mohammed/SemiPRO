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
            Logger::getInstance().log("No config file specified, running default simulation");

            // Default simulation for testing
            auto& engine = SimulationEngine::getInstance();
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
        auto wafer = engine.createWafer(300.0, 775.0, "silicon");
        wafer->initializeGrid(50, 50);
        engine.registerWafer(wafer, "main_wafer");

        // Set up process parameters
        SimulationEngine::ProcessParameters params(process_type, 1.0);

        // Parse parameters based on process type
        if (process_type == "oxidation") {
            params.parameters["temperature"] = std::stod(config.count("temperature") ? config["temperature"] : "1000");
            params.parameters["time"] = std::stod(config.count("time") ? config["time"] : "1.0");
            params.parameters["ambient"] = config.count("atmosphere") && config["atmosphere"] == "wet" ? 1.0 : 0.0;
        } else if (process_type == "doping") {
            params.parameters["energy"] = std::stod(config.count("energy") ? config["energy"] : "50");
            params.parameters["dose"] = std::stod(config.count("concentration") ? config["concentration"] : "1e15");
            params.parameters["mass"] = 11.0; // Default: boron
            params.parameters["atomic_number"] = 5.0; // Default: boron
        } else if (process_type == "deposition") {
            params.parameters["thickness"] = std::stod(config.count("thickness") ? config["thickness"] : "0.5");
            params.parameters["temperature"] = std::stod(config.count("temperature") ? config["temperature"] : "400");
        }

        // Execute simulation
        auto future = engine.simulateProcessAsync("main_wafer", params);
        bool success = future.get();

        Logger::getInstance().log("Simulation completed: " + process_type + " - " +
                                 std::string(success ? "SUCCESS" : "FAILED"));

        return success ? 0 : 1;

    } catch (const std::exception& e) {
        Logger::getInstance().log("Simulation failed with exception: " + std::string(e.what()));
        return 1;
    }
}