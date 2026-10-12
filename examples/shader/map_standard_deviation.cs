#version 430 core

layout(binding = 0, r32ui) uniform uimage2D counterWritten;
layout(binding = 1, rgba32f) uniform image2D squaredDeviation;

layout(local_size_x = 1, local_size_y = 1) in;

void main() {
    ivec2 envCoord = ivec2(gl_GlobalInvocationID.xy);

    vec4 sD = imageLoad(squaredDeviation, envCoord);
    uint counter = imageLoad(counterWritten, envCoord).r;
    vec4 Deviation = sqrt(sD / counter);
    imageStore(squaredDeviation, envCoord, Deviation);
}
