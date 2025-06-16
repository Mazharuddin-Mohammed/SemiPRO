#version 450

// Author: Dr. Mazharuddin Mohammed
// Fragment shader for wafer visualization
// Supports advanced lighting, material properties, and visual effects

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

layout(binding = 2) uniform MaterialProperties {
    vec4 ambient[16];
    vec4 diffuse[16];
    vec4 specular[16];
    float shininess[16];
    float metallic[16];
    float roughness[16];
    float transparency[16];
    float refractiveIndex[16];
} materials;

layout(binding = 3) uniform sampler2D textureSampler;
layout(binding = 4) uniform sampler2D normalMap;
layout(binding = 5) uniform sampler2D roughnessMap;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragColor;
layout(location = 4) in float fragLayerIndex;
layout(location = 5) in vec3 fragLightPos;
layout(location = 6) in vec3 fragViewPos;

layout(location = 0) out vec4 outColor;

// Lighting calculation functions
vec3 calculatePhongLighting(vec3 normal, vec3 lightDir, vec3 viewDir, vec3 color) {
    // Ambient
    vec3 ambient = 0.15 * color;
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * color;
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * vec3(0.5);
    
    return ambient + diffuse + specular;
}

vec3 calculatePBRLighting(vec3 normal, vec3 lightDir, vec3 viewDir, vec3 albedo, 
                         float metallic, float roughness) {
    // Simplified PBR lighting model
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // Normal Distribution Function (GGX/Trowbridge-Reitz)
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float NdotH = max(dot(normal, halfwayDir), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = NdotH2 * (alpha2 - 1.0) + 1.0;
    float D = alpha2 / (3.14159265 * denom * denom);
    
    // Geometry function (Smith's method)
    float NdotV = max(dot(normal, viewDir), 0.0);
    float NdotL = max(dot(normal, lightDir), 0.0);
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float G1L = NdotL / (NdotL * (1.0 - k) + k);
    float G1V = NdotV / (NdotV * (1.0 - k) + k);
    float G = G1L * G1V;
    
    // Fresnel equation (Schlick's approximation)
    float cosTheta = max(dot(halfwayDir, viewDir), 0.0);
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
    
    // Cook-Torrance BRDF
    vec3 numerator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.001;
    vec3 specular = numerator / denominator;
    
    // Energy conservation
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    vec3 diffuse = kD * albedo / 3.14159265;
    
    return (diffuse + specular) * NdotL;
}

// Material-specific effects
vec3 applySiliconEffect(vec3 baseColor, vec2 texCoord) {
    // Simulate silicon crystal structure visualization
    float pattern = sin(texCoord.x * 100.0) * sin(texCoord.y * 100.0);
    return baseColor + vec3(0.1) * pattern;
}

vec3 applyMetalEffect(vec3 baseColor, vec3 normal, vec3 viewDir) {
    // Simulate metallic reflection
    float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), 2.0);
    return mix(baseColor, vec3(1.0), fresnel * 0.3);
}

vec3 applyOxideEffect(vec3 baseColor, float thickness) {
    // Simulate thin film interference for oxide layers
    float interference = sin(thickness * 10.0) * 0.2;
    return baseColor + vec3(interference, interference * 0.5, -interference * 0.5);
}

// Visual effects for process simulation
vec3 applyTemperatureEffect(vec3 baseColor, float temperature) {
    // Map temperature to color (blue = cold, red = hot)
    float normalizedTemp = clamp(temperature / 1000.0, 0.0, 1.0);
    vec3 tempColor = mix(vec3(0.0, 0.0, 1.0), vec3(1.0, 0.0, 0.0), normalizedTemp);
    return mix(baseColor, tempColor, 0.3);
}

vec3 applyStressEffect(vec3 baseColor, float stress) {
    // Visualize stress with color intensity
    float normalizedStress = clamp(abs(stress) / 1000.0, 0.0, 1.0);
    vec3 stressColor = stress > 0.0 ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 0.0, 1.0);
    return mix(baseColor, stressColor, normalizedStress * 0.5);
}

// Defect visualization
vec3 applyDefectHighlight(vec3 baseColor, vec2 texCoord) {
    // Highlight defects with pulsing effect
    float pulse = sin(ubo.time * 5.0) * 0.5 + 0.5;
    float defectMask = texture(textureSampler, texCoord).r;
    vec3 defectColor = vec3(1.0, 0.0, 0.0) * pulse;
    return mix(baseColor, defectColor, defectMask);
}

void main() {
    // Normalize inputs
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(fragLightPos - fragPos);
    vec3 viewDir = normalize(fragViewPos - fragPos);
    
    // Get layer-specific properties
    int layerIdx = int(fragLayerIndex);
    vec3 baseColor = fragColor;
    float metallic = 0.0;
    float roughness = 0.5;
    float transparency = 1.0;
    
    if (layerIdx < 16) {
        metallic = materials.metallic[layerIdx];
        roughness = materials.roughness[layerIdx];
        transparency = materials.transparency[layerIdx];
    }
    
    // Apply material-specific effects
    if (layerIdx < layers.numLayers) {
        float layerThickness = layers.layerThicknesses[layerIdx];
        float layerProperty = layers.layerProperties[layerIdx];
        
        // Determine material type and apply appropriate effects
        if (baseColor.r > 0.8 && baseColor.g < 0.3) {
            // Reddish - likely metal
            baseColor = applyMetalEffect(baseColor, normal, viewDir);
        } else if (baseColor.b > 0.8) {
            // Bluish - likely oxide
            baseColor = applyOxideEffect(baseColor, layerThickness);
        } else if (baseColor.g > 0.5 && baseColor.r < 0.3) {
            // Greenish - likely silicon
            baseColor = applySiliconEffect(baseColor, fragTexCoord);
        }
        
        // Apply property-based effects
        if (ubo.renderMode == 2) {
            baseColor = applyTemperatureEffect(baseColor, layerProperty);
        } else if (ubo.renderMode == 3) {
            baseColor = applyStressEffect(baseColor, layerProperty);
        }
    }
    
    // Calculate lighting
    vec3 finalColor;
    if (metallic > 0.5) {
        // Use PBR for metallic materials
        finalColor = calculatePBRLighting(normal, lightDir, viewDir, baseColor, metallic, roughness);
    } else {
        // Use Phong for non-metallic materials
        finalColor = calculatePhongLighting(normal, lightDir, viewDir, baseColor);
    }
    
    // Apply special effects
    if (ubo.renderMode == 4) {
        // Defect visualization mode
        finalColor = applyDefectHighlight(finalColor, fragTexCoord);
    }
    
    // Apply time-based animation effects
    if (ubo.time > 0.0) {
        float wave = sin(fragPos.x * 0.1 + ubo.time) * sin(fragPos.y * 0.1 + ubo.time);
        finalColor += vec3(0.1) * wave;
    }
    
    // Apply transparency
    float alpha = transparency;
    if (ubo.renderMode == 5) {
        // X-ray mode - make everything semi-transparent
        alpha *= 0.3;
    }
    
    // Gamma correction
    finalColor = pow(finalColor, vec3(1.0/2.2));
    
    outColor = vec4(finalColor, alpha);
}

// Utility functions for advanced effects
float calculateFresnel(vec3 normal, vec3 viewDir, float ior) {
    float cosTheta = max(dot(normal, viewDir), 0.0);
    float r0 = pow((1.0 - ior) / (1.0 + ior), 2.0);
    return r0 + (1.0 - r0) * pow(1.0 - cosTheta, 5.0);
}

vec3 calculateSubsurfaceScattering(vec3 normal, vec3 lightDir, vec3 viewDir, 
                                  vec3 color, float thickness) {
    // Simplified subsurface scattering for thin films
    float backlight = max(0.0, dot(-normal, lightDir));
    float scattering = pow(backlight, 2.0) * thickness * 0.1;
    return color + vec3(scattering);
}
