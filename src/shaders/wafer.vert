#version 450

// Author: Dr. Mazharuddin Mohammed
// Vertex shader for wafer visualization
// Supports 3D wafer rendering with layer information

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 lightPos;
    vec3 viewPos;
    float time;
    float layerHeight;
    int renderMode;
    float materialIndex;
} ubo;

layout(binding = 1) uniform LayerData {
    vec4 layerColors[32];
    float layerThicknesses[32];
    float layerProperties[32];
    int numLayers;
} layers;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in float inLayerIndex;
layout(location = 4) in vec3 inColor;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragColor;
layout(location = 4) out float fragLayerIndex;
layout(location = 5) out vec3 fragLightPos;
layout(location = 6) out vec3 fragViewPos;

void main() {
    // Calculate world position
    vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
    
    // Apply layer-specific height offset
    int layerIdx = int(inLayerIndex);
    float heightOffset = 0.0;
    
    // Calculate cumulative height for layers below current layer
    for (int i = 0; i < layerIdx && i < layers.numLayers; i++) {
        heightOffset += layers.layerThicknesses[i];
    }
    
    // Apply height offset to z-coordinate
    worldPos.z += heightOffset * ubo.layerHeight;
    
    // Transform to clip space
    gl_Position = ubo.proj * ubo.view * worldPos;
    
    // Pass data to fragment shader
    fragPos = worldPos.xyz;
    fragNormal = mat3(transpose(inverse(ubo.model))) * inNormal;
    fragTexCoord = inTexCoord;
    fragLayerIndex = inLayerIndex;
    fragLightPos = ubo.lightPos;
    fragViewPos = ubo.viewPos;
    
    // Determine fragment color based on render mode
    if (ubo.renderMode == 0) {
        // Material-based coloring
        if (layerIdx < layers.numLayers) {
            fragColor = layers.layerColors[layerIdx].rgb;
        } else {
            fragColor = inColor;
        }
    } else if (ubo.renderMode == 1) {
        // Layer-based coloring
        float normalizedLayer = float(layerIdx) / float(max(layers.numLayers - 1, 1));
        fragColor = vec3(normalizedLayer, 1.0 - normalizedLayer, 0.5);
    } else if (ubo.renderMode == 2) {
        // Property-based coloring (e.g., stress, temperature)
        if (layerIdx < layers.numLayers) {
            float property = layers.layerProperties[layerIdx];
            // Map property to color (red = high, blue = low)
            fragColor = vec3(property, 0.0, 1.0 - property);
        } else {
            fragColor = vec3(0.5, 0.5, 0.5);
        }
    } else {
        // Default coloring
        fragColor = inColor;
    }
    
    // Apply time-based effects for animation
    if (ubo.time > 0.0) {
        float pulse = sin(ubo.time * 2.0) * 0.1 + 0.9;
        fragColor *= pulse;
    }
}

// Additional vertex processing for special effects
vec3 calculateWaveDeformation(vec3 position, float time) {
    vec3 deformed = position;
    
    // Apply wave deformation for process simulation
    float wave = sin(position.x * 0.1 + time) * sin(position.y * 0.1 + time) * 0.01;
    deformed.z += wave;
    
    return deformed;
}

vec3 calculateStressDeformation(vec3 position, float stress) {
    vec3 deformed = position;
    
    // Apply stress-based deformation
    float deformation = stress * 0.001; // Scale factor
    deformed.z += deformation;
    
    return deformed;
}

// Utility functions for coordinate transformations
vec2 cartesianToPolar(vec2 cartesian) {
    float r = length(cartesian);
    float theta = atan(cartesian.y, cartesian.x);
    return vec2(r, theta);
}

vec3 applyWaferCurvature(vec3 position, float curvature) {
    vec3 curved = position;
    float r = length(position.xy);
    curved.z += curvature * r * r;
    return curved;
}
