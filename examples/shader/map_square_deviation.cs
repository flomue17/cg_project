#version 430 core

layout(binding = 0, rgba8) uniform image2D u_inputImage;  // Eingabebild
layout(binding = 1, rgba32f) uniform image2D u_mean;      // Environment Map
layout(binding = 2, rgba32f) uniform image2D squaredDeviation;

uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;

uniform ivec2 u_imageResolution;
uniform ivec2 u_envMapResolution;

uniform int useMasks;

const float M_PI = 3.1415926535897932384626433832795;

// Konvertiere Koordinaten von Environment-Map nach Weltkoordinaten
vec3 envMapToWorldCoords(vec2 coords) {
    float phi = coords.x * 2 * M_PI;
    float theta = coords.y * M_PI;

    float sinTheta = sin(theta);
    float cosTheta = cos(theta);
    float sinPhi = sin(phi);
    float cosPhi = cos(phi);

    return vec3(-cosPhi * sinTheta, sinPhi * sinTheta, -cosTheta);
}

layout(local_size_x = 1, local_size_y = 1) in;

void main() {
    ivec2 envCoord = ivec2(gl_GlobalInvocationID.xy);

    vec2 envMapCoords = (vec2(envCoord) + vec2(0.5)) / vec2(u_envMapResolution);
    vec3 worldPos = envMapToWorldCoords(envMapCoords);  // Environment-Map-Koordinate -> Weltposition

    vec4 clipSpacePos = u_ProjectionMatrix * u_ViewMatrix * vec4(worldPos, 1);

    if (clipSpacePos.z <= 0) {
        return;
    }

    clipSpacePos = clipSpacePos / clipSpacePos.w;

    ivec2 screen_position = ivec2((clipSpacePos.xy + 1) * 0.5 * u_imageResolution);

    if (screen_position.x < 0 || screen_position.y < 0 || screen_position.x >= u_imageResolution.x || screen_position.y >= u_imageResolution.y) {
        return;
    }

    vec4 color = imageLoad(u_inputImage, screen_position);

    if ((useMasks == 0) || color.w == 1) {
        vec4 deviation = abs(color - imageLoad(u_mean, envCoord));
        deviation *= deviation;

        vec4 sumValue = imageLoad(squaredDeviation, envCoord);
        imageStore(squaredDeviation, envCoord, deviation + sumValue);
    }
}
