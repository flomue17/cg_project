#version 430 core

layout(binding = 0, rgba32f) uniform image2D envMap;
layout(binding = 1, rgba32f) uniform image2D filteredEnvMap;
layout(binding = 2, r32f) uniform image2D confidence;

uniform float threshold;

layout(local_size_x = 1, local_size_y = 1) in;


void main() {
    ivec2 envCoord = ivec2(gl_GlobalInvocationID.xy);

    float confidenceEnvMap = imageLoad(confidence, envCoord).r;

    vec4 color;
    if(confidenceEnvMap < threshold){
        color = vec4(0,0,0,1);
    } else {
        color = imageLoad(envMap,envCoord);
    }

    imageStore(filteredEnvMap, envCoord, color);
}