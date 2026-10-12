#version 430 core

layout(binding = 0, rgba8) uniform image2D u_inputImage; //sampler2D
layout(binding = 1, rgba32f) uniform image2D u_envMap;

uniform mat4 u_invViewMatrix;
uniform mat4 u_invProjectionMatrix;

uniform ivec2 u_imageResolution;
uniform ivec2 u_envMapResolution;

const float M_PI = 3.1415926535897932384626433832795;

vec2 worldToEnvMapCoords(vec3 worldPos) {
    worldPos = vec3(-worldPos.x, worldPos.y, -worldPos.z);
    float r = length(worldPos);           // Berechne den Radius
    float phi = atan(worldPos.y, worldPos.x);                // Berechne den Azimutwinkel
    float theta = acos(worldPos.z / r);             // Berechne den Polwinkel

    float u = phi / (2.0 * M_PI); // Normalisiere phi zu [0, 1]

    // Berechne die Texturkoordinate v (Polwinkel, theta)
    float v = theta / M_PI; // Normalisiere theta zu [0, 1]



    return vec2(mod(u+2,1), v);
}

layout(local_size_x = 1, local_size_y = 1) in;

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);

    // Von Bildschirmpixeln in NDC
    vec2 texCoord = vec2(pixelCoord) / vec2(u_imageResolution); // [0, 1]
    vec4 clipSpace = vec4(texCoord * 2.0 - 1.0, 0.8, 1.0); // [0, 1] -> [-1, 1]

    vec4 worldSpace = u_invViewMatrix * u_invProjectionMatrix * clipSpace;
    worldSpace /= worldSpace.w;

    vec2 envMapCoords = worldToEnvMapCoords(worldSpace.xyz);
    ivec2 envPixelCoord = ivec2(envMapCoords * vec2(u_envMapResolution));

    vec4 color = imageLoad(u_inputImage, pixelCoord);
    if (color.w == 1){
        imageStore(u_envMap, envPixelCoord, color);
    }
}

