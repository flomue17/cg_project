#version 430 core

layout(binding = 0, rgba32f) uniform image3D buckets;

layout(binding = 1, r32ui) uniform uimage3D bucketCounters;

layout(binding = 2, rgba32f) uniform image2D envMap;

layout(local_size_x = 1, local_size_y = 1) in;

void main() {
    ivec2 envCoord = ivec2(gl_GlobalInvocationID.xy);

    uint counter[8];
    float sumCounter = 0;


    int from = 0;
    int to = 8;

    for(int i = from; i < to; i++) {
        counter[i] = imageLoad(bucketCounters, ivec3(envCoord, i)).r;
        sumCounter += counter[i];
    }

    if(sumCounter == 0) {
//         imageStore(envMap, envCoord, vec4(1,0,1,1));
        return;
    }

    vec4 color = vec4(0,0,0,1);

    for(int i = from; i < to; i++) {
        color += (1.0f / sumCounter) * imageLoad(buckets, ivec3(envCoord, i));
    }

//     color /= sumCounter;
    imageStore(envMap, envCoord, color);
}