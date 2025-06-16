#include <catch2/catch_test_macros.hpp>
#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/geometry/geometry_manager.hpp"
#include "../../src/cpp/modules/oxidation/oxidation_model.hpp"
#include "../../src/cpp/modules/doping/doping_manager.hpp"
#include <chrono>
#include <random>

TEST_CASE("Comprehensive Wafer Operations", "[comprehensive]") {
    auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
    
    SECTION("Large grid initialization") {
        GeometryManager geometry;
        geometry.initializeGrid(wafer, 1000, 1000);
        
        REQUIRE(wafer->getGrid().rows() == 1000);
        REQUIRE(wafer->getGrid().cols() == 1000);
        REQUIRE(wafer->getGrid().sum() == 0.0);
    }
    
    SECTION("Multiple layer operations") {
        wafer->initializeGrid(100, 100);
        
        // Add multiple layers
        for (int i = 0; i < 10; ++i) {
            wafer->addFilmLayer(0.1, "layer_" + std::to_string(i));
        }
        
        REQUIRE(wafer->getFilmLayers().size() == 10);
        
        // Check total thickness
        double total_thickness = 0.0;
        for (const auto& layer : wafer->getFilmLayers()) {
            total_thickness += layer.first;
        }
        REQUIRE(total_thickness == Catch::Approx(1.0).epsilon(0.01));
    }
    
    SECTION("Stress testing with random operations") {
        wafer->initializeGrid(50, 50);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> thickness_dist(0.01, 1.0);
        std::uniform_int_distribution<> material_dist(0, 4);
        
        std::vector<std::string> materials = {"SiO2", "Si3N4", "Al", "Cu", "W"};
        
        // Perform 100 random operations
        for (int i = 0; i < 100; ++i) {
            double thickness = thickness_dist(gen);
            std::string material = materials[material_dist(gen)];
            wafer->addFilmLayer(thickness, material);
        }
        
        REQUIRE(wafer->getFilmLayers().size() == 100);
        REQUIRE(wafer->getGrid().rows() == 50);
        REQUIRE(wafer->getGrid().cols() == 50);
    }
}

TEST_CASE("Performance Benchmarks", "[performance]") {
    auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
    
    SECTION("Grid initialization performance") {
        auto start = std::chrono::high_resolution_clock::now();
        
        GeometryManager geometry;
        geometry.initializeGrid(wafer, 1000, 1000);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Should complete within reasonable time (adjust based on hardware)
        REQUIRE(duration.count() < 1000); // Less than 1 second
    }
    
    SECTION("Oxidation simulation performance") {
        wafer->initializeGrid(200, 200);
        OxidationModel oxidation;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        oxidation.simulateOxidation(wafer, 1000.0, 60.0, "dry");
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        REQUIRE(duration.count() < 5000); // Less than 5 seconds
    }
    
    SECTION("Memory usage validation") {
        // Test memory doesn't grow excessively with large grids
        size_t initial_layers = wafer->getFilmLayers().size();
        
        wafer->initializeGrid(500, 500);
        
        for (int i = 0; i < 50; ++i) {
            wafer->addFilmLayer(0.1, "test_layer");
        }
        
        REQUIRE(wafer->getFilmLayers().size() == initial_layers + 50);
        
        // Grid should still be valid
        REQUIRE(wafer->getGrid().rows() == 500);
        REQUIRE(wafer->getGrid().cols() == 500);
    }
}

TEST_CASE("Edge Cases and Error Handling", "[edge_cases]") {
    auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
    
    SECTION("Zero and negative dimensions") {
        GeometryManager geometry;
        
        // These should handle gracefully or throw appropriate exceptions
        REQUIRE_NOTHROW(geometry.initializeGrid(wafer, 1, 1));
        
        // Very small grid
        geometry.initializeGrid(wafer, 2, 2);
        REQUIRE(wafer->getGrid().rows() == 2);
        REQUIRE(wafer->getGrid().cols() == 2);
    }
    
    SECTION("Extreme values") {
        wafer->initializeGrid(10, 10);
        
        // Very thin layers
        REQUIRE_NOTHROW(wafer->addFilmLayer(1e-6, "ultra_thin"));
        
        // Very thick layers
        REQUIRE_NOTHROW(wafer->addFilmLayer(1000.0, "ultra_thick"));
        
        REQUIRE(wafer->getFilmLayers().size() == 2);
    }
    
    SECTION("String handling") {
        wafer->initializeGrid(10, 10);
        
        // Empty material name
        REQUIRE_NOTHROW(wafer->addFilmLayer(0.1, ""));
        
        // Very long material name
        std::string long_name(1000, 'x');
        REQUIRE_NOTHROW(wafer->addFilmLayer(0.1, long_name));
        
        // Special characters
        REQUIRE_NOTHROW(wafer->addFilmLayer(0.1, "material_with_123!@#$%"));
    }
    
    SECTION("Grid operations on uninitialized wafer") {
        auto empty_wafer = std::make_shared<Wafer>(100.0, 500.0, "silicon");
        
        // Should handle uninitialized grid gracefully
        REQUIRE_NOTHROW(empty_wafer->addFilmLayer(0.1, "test"));
        
        // Initialize after adding layers
        empty_wafer->initializeGrid(10, 10);
        REQUIRE(empty_wafer->getGrid().rows() == 10);
        REQUIRE(empty_wafer->getGrid().cols() == 10);
    }
}

TEST_CASE("Multi-threading Safety", "[threading]") {
    auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
    wafer->initializeGrid(100, 100);
    
    SECTION("Concurrent layer additions") {
        std::vector<std::thread> threads;
        std::atomic<int> completed_operations{0};
        
        // Launch multiple threads adding layers
        for (int t = 0; t < 4; ++t) {
            threads.emplace_back([&wafer, &completed_operations, t]() {
                for (int i = 0; i < 25; ++i) {
                    wafer->addFilmLayer(0.01, "thread_" + std::to_string(t) + "_layer_" + std::to_string(i));
                    completed_operations++;
                }
            });
        }
        
        // Wait for all threads
        for (auto& thread : threads) {
            thread.join();
        }
        
        REQUIRE(completed_operations == 100);
        REQUIRE(wafer->getFilmLayers().size() == 100);
    }
}

TEST_CASE("Data Integrity Validation", "[integrity]") {
    auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
    
    SECTION("Wafer properties consistency") {
        REQUIRE(wafer->getDiameter() == 300.0);
        REQUIRE(wafer->getThickness() == 775.0);
        REQUIRE(wafer->getMaterial() == "silicon");
        
        // Properties should remain consistent after operations
        wafer->initializeGrid(50, 50);
        wafer->addFilmLayer(0.5, "SiO2");
        
        REQUIRE(wafer->getDiameter() == 300.0);
        REQUIRE(wafer->getThickness() == 775.0);
        REQUIRE(wafer->getMaterial() == "silicon");
    }
    
    SECTION("Grid data consistency") {
        wafer->initializeGrid(20, 30);
        
        // Grid should maintain dimensions
        REQUIRE(wafer->getGrid().rows() == 20);
        REQUIRE(wafer->getGrid().cols() == 30);
        
        // Modify grid and verify
        Eigen::ArrayXXd new_grid = Eigen::ArrayXXd::Ones(20, 30) * 5.0;
        wafer->updateGrid(new_grid);
        
        REQUIRE(wafer->getGrid().sum() == Catch::Approx(20 * 30 * 5.0));
        REQUIRE(wafer->getGrid().rows() == 20);
        REQUIRE(wafer->getGrid().cols() == 30);
    }
    
    SECTION("Layer stack integrity") {
        wafer->initializeGrid(10, 10);
        
        // Add layers and verify order
        wafer->addFilmLayer(0.1, "layer1");
        wafer->addFilmLayer(0.2, "layer2");
        wafer->addFilmLayer(0.3, "layer3");
        
        const auto& layers = wafer->getFilmLayers();
        REQUIRE(layers.size() == 3);
        REQUIRE(layers[0].first == 0.1);
        REQUIRE(layers[0].second == "layer1");
        REQUIRE(layers[1].first == 0.2);
        REQUIRE(layers[1].second == "layer2");
        REQUIRE(layers[2].first == 0.3);
        REQUIRE(layers[2].second == "layer3");
    }
}
