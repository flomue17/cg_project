#version 430 core

layout(binding = 0, rgba8) uniform image2D u_inputImage;  // Eingabebild
layout(binding = 1, rgba32f) uniform image2D u_envMap;
layout(binding = 2, r32f) uniform image2D u_confidence;

uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;

uniform ivec2 u_imageResolution;
uniform ivec2 u_envMapResolution;

uniform float confidenceThreshold;
uniform float confidenceIncreaseFactor;
uniform float confidenceDecreaseFactor;
uniform float initialConfidence;

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
    vec3 worldPos = envMapToWorldCoords(envMapCoords);

    vec4 clipSpacePos = u_ProjectionMatrix * u_ViewMatrix * vec4(worldPos, 1);

    if (clipSpacePos.z <= 0) {
        return;
    }

    clipSpacePos = clipSpacePos / clipSpacePos.w;

    ivec2 screen_position = ivec2((clipSpacePos.xy + 1) * 0.5 * u_imageResolution);

    if (screen_position.x < 0 || screen_position.y < 0 ||
        screen_position.x >= u_imageResolution.x || screen_position.y >= u_imageResolution.y) {
        return;
    }

    vec4 color = imageLoad(u_inputImage, screen_position);

    if ((useMasks == 0) || color.w == 1) {
        vec4 envValue = imageLoad(u_envMap, envCoord);
        vec4 confidence = imageLoad(u_confidence, envCoord);

        // Initialisiere, wenn erster Treffer
        if (envValue.w == 0.0) {
            imageStore(u_envMap, envCoord, vec4(color.xyz, 1.0));
            imageStore(u_confidence, envCoord, vec4(initialConfidence, 0, 0, 0));
        } else {
            float delta = length(color.rgb - envValue.rgb);

            float w_old;
            float w_new;

            float w_matze = confidence.r;
            if (delta < confidenceThreshold) { // value close -> increase confidence
//                 confidence.r = min(1.0, confidence.r + confidenceIncreaseFactor * (exp(-delta)));//(1.73205-delta));
                w_old = confidence.r;
                w_new = confidenceIncreaseFactor * (exp(-delta));
                confidence.r = confidence.r + confidenceIncreaseFactor * (exp(-delta));//(1.73205-delta));
            } else {
//                 confidence.r = max(0, confidence.r - confidenceDecreaseFactor * (exp(-delta)));//(1.73205-delta));
                w_new = confidenceDecreaseFactor * (exp(-delta));
                confidence.r = confidence.r - confidenceDecreaseFactor * (exp(-delta));//(1.73205-delta));
                w_old = confidence.r;
            }

            float sum = w_new + w_old;
            w_old /= sum;
            w_new /= sum;

            w_old = -exp(-w_matze)+1.f;
            w_new = 1.f - w_old;

            if(w_matze < 0.001f){
                w_old = 0.f;
                w_new = 1.f;
                confidence.r = confidenceIncreaseFactor;
            }



            envValue.rgb = envValue.rgb * w_old + color.rgb * w_new;

            envValue.rgb = clamp(envValue.rgb, 0.0, 1.0);

            //envValue.a = confidence.r/3.f;
            imageStore(u_envMap, envCoord, envValue);
            imageStore(u_confidence, envCoord, confidence);
        }
    }
}
