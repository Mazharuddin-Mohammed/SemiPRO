#!/bin/bash

# Shader compilation script for SemiPRO
# Compiles GLSL shaders to SPIR-V using glslc

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Directories
SHADER_DIR="src/shaders"
OUTPUT_DIR="src/shaders/compiled"

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

echo -e "${GREEN}SemiPRO Shader Compilation${NC}"
echo "=================================="

# Check if glslc is available
if ! command -v glslc &> /dev/null; then
    echo -e "${RED}Error: glslc not found. Please install the Vulkan SDK.${NC}"
    echo "Download from: https://vulkan.lunarg.com/"
    exit 1
fi

echo -e "${GREEN}Found glslc:${NC} $(which glslc)"
echo -e "${GREEN}Version:${NC} $(glslc --version | head -n1)"
echo ""

# Function to compile a shader
compile_shader() {
    local shader_file="$1"
    local shader_name=$(basename "$shader_file")
    local output_file="$OUTPUT_DIR/${shader_name}.spv"
    
    echo -n "Compiling $shader_name... "
    
    if glslc "$shader_file" -o "$output_file" 2>/dev/null; then
        echo -e "${GREEN}✓${NC}"
        
        # Get file size
        local size=$(stat -c%s "$output_file" 2>/dev/null || stat -f%z "$output_file" 2>/dev/null || echo "unknown")
        echo "  Output: $output_file ($size bytes)"
    else
        echo -e "${RED}✗${NC}"
        echo -e "${RED}Error compiling $shader_name:${NC}"
        glslc "$shader_file" -o "$output_file"
        return 1
    fi
}

# Function to validate shader
validate_shader() {
    local shader_file="$1"
    local shader_name=$(basename "$shader_file")
    
    echo -n "Validating $shader_name... "
    
    if glslc --target-env=vulkan1.0 -fshader-stage=vertex "$shader_file" -S 2>/dev/null >/dev/null; then
        echo -e "${GREEN}✓${NC}"
    else
        echo -e "${YELLOW}Warning: Validation issues${NC}"
    fi
}

# Compile vertex shaders
echo -e "${YELLOW}Compiling Vertex Shaders:${NC}"
for shader in "$SHADER_DIR"/*.vert; do
    if [ -f "$shader" ]; then
        compile_shader "$shader"
        validate_shader "$shader"
        echo ""
    fi
done

# Compile fragment shaders
echo -e "${YELLOW}Compiling Fragment Shaders:${NC}"
for shader in "$SHADER_DIR"/*.frag; do
    if [ -f "$shader" ]; then
        compile_shader "$shader"
        validate_shader "$shader"
        echo ""
    fi
done

# Compile compute shaders
echo -e "${YELLOW}Compiling Compute Shaders:${NC}"
for shader in "$SHADER_DIR"/*.comp; do
    if [ -f "$shader" ]; then
        compile_shader "$shader"
        echo ""
    fi
done

# Compile geometry shaders
echo -e "${YELLOW}Compiling Geometry Shaders:${NC}"
for shader in "$SHADER_DIR"/*.geom; do
    if [ -f "$shader" ]; then
        compile_shader "$shader"
        echo ""
    fi
done

# Generate shader header file for C++ inclusion
echo -e "${YELLOW}Generating C++ Header:${NC}"
HEADER_FILE="src/cpp/renderer/compiled_shaders.hpp"

cat > "$HEADER_FILE" << 'EOF'
#ifndef COMPILED_SHADERS_HPP
#define COMPILED_SHADERS_HPP

#include <vector>
#include <string>
#include <unordered_map>

class CompiledShaders {
public:
    static std::vector<char> loadShader(const std::string& filename);
    static std::unordered_map<std::string, std::string> getShaderPaths();
    
private:
    static const std::unordered_map<std::string, std::string> shader_paths_;
};

EOF

# Add shader paths to header
echo "const std::unordered_map<std::string, std::string> CompiledShaders::shader_paths_ = {" >> "$HEADER_FILE"

for spv_file in "$OUTPUT_DIR"/*.spv; do
    if [ -f "$spv_file" ]; then
        shader_name=$(basename "$spv_file" .spv)
        echo "    {\"$shader_name\", \"$spv_file\"}," >> "$HEADER_FILE"
    fi
done

echo "};" >> "$HEADER_FILE"
echo "" >> "$HEADER_FILE"
echo "#endif // COMPILED_SHADERS_HPP" >> "$HEADER_FILE"

echo "Generated: $HEADER_FILE"
echo ""

# Summary
echo -e "${GREEN}Compilation Summary:${NC}"
echo "==================="

total_shaders=$(find "$OUTPUT_DIR" -name "*.spv" | wc -l)
total_size=$(find "$OUTPUT_DIR" -name "*.spv" -exec stat -c%s {} + 2>/dev/null | awk '{sum+=$1} END {print sum}' || echo "0")

echo "Total shaders compiled: $total_shaders"
echo "Total size: $total_size bytes"
echo "Output directory: $OUTPUT_DIR"

# Check for any missing shaders
echo ""
echo -e "${YELLOW}Shader Inventory:${NC}"
for shader_type in vert frag comp geom; do
    count=$(find "$SHADER_DIR" -name "*.$shader_type" | wc -l)
    compiled_count=$(find "$OUTPUT_DIR" -name "*.$shader_type.spv" | wc -l)
    echo "$shader_type shaders: $compiled_count/$count compiled"
done

echo ""
echo -e "${GREEN}Shader compilation complete!${NC}"

# Optional: Run shader validation
if command -v spirv-val &> /dev/null; then
    echo ""
    echo -e "${YELLOW}Running SPIR-V Validation:${NC}"
    for spv_file in "$OUTPUT_DIR"/*.spv; do
        if [ -f "$spv_file" ]; then
            shader_name=$(basename "$spv_file")
            echo -n "Validating $shader_name... "
            if spirv-val "$spv_file" 2>/dev/null; then
                echo -e "${GREEN}✓${NC}"
            else
                echo -e "${RED}✗${NC}"
            fi
        fi
    done
fi

# Create a makefile target for easy recompilation
cat > "Makefile.shaders" << 'EOF'
# Shader compilation makefile
# Usage: make -f Makefile.shaders

SHADER_DIR = src/shaders
OUTPUT_DIR = src/shaders/compiled
GLSLC = glslc

# Find all shader files
VERT_SHADERS = $(wildcard $(SHADER_DIR)/*.vert)
FRAG_SHADERS = $(wildcard $(SHADER_DIR)/*.frag)
COMP_SHADERS = $(wildcard $(SHADER_DIR)/*.comp)
GEOM_SHADERS = $(wildcard $(SHADER_DIR)/*.geom)

# Generate output file names
VERT_SPV = $(VERT_SHADERS:$(SHADER_DIR)/%.vert=$(OUTPUT_DIR)/%.vert.spv)
FRAG_SPV = $(FRAG_SHADERS:$(SHADER_DIR)/%.frag=$(OUTPUT_DIR)/%.frag.spv)
COMP_SPV = $(COMP_SHADERS:$(SHADER_DIR)/%.comp=$(OUTPUT_DIR)/%.comp.spv)
GEOM_SPV = $(GEOM_SHADERS:$(SHADER_DIR)/%.geom=$(OUTPUT_DIR)/%.geom.spv)

ALL_SPV = $(VERT_SPV) $(FRAG_SPV) $(COMP_SPV) $(GEOM_SPV)

.PHONY: all clean

all: $(ALL_SPV)

$(OUTPUT_DIR)/%.vert.spv: $(SHADER_DIR)/%.vert
	@mkdir -p $(OUTPUT_DIR)
	$(GLSLC) $< -o $@

$(OUTPUT_DIR)/%.frag.spv: $(SHADER_DIR)/%.frag
	@mkdir -p $(OUTPUT_DIR)
	$(GLSLC) $< -o $@

$(OUTPUT_DIR)/%.comp.spv: $(SHADER_DIR)/%.comp
	@mkdir -p $(OUTPUT_DIR)
	$(GLSLC) $< -o $@

$(OUTPUT_DIR)/%.geom.spv: $(SHADER_DIR)/%.geom
	@mkdir -p $(OUTPUT_DIR)
	$(GLSLC) $< -o $@

clean:
	rm -rf $(OUTPUT_DIR)

EOF

echo ""
echo "Created Makefile.shaders for easy recompilation"
echo "Usage: make -f Makefile.shaders"
