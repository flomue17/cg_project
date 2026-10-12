#version 430 core

layout(binding = 0, rgba8) uniform image2D u_inputImage;

layout(binding = 1, rgba32f) uniform image3D buckets;

layout(binding = 2, r32ui) uniform uimage3D bucketCounters;

layout(binding = 3, rgba8) uniform image2D kmeans;

uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;

uniform ivec2 u_imageResolution;
uniform ivec2 u_envMapResolution;

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

// Funktion zum Berechnen des euklidischen Abstands zwischen zwei Farben
float colorDistance(vec3 color1, vec3 color2) {
    return length(color1 - color2);  // euklidischer Abstand im RGB-Raum
}

// Überprüfen, ob die Farbe bereits in einem der Buckets existiert
int getSimilarBucketIndex(vec3 color, ivec2 envCoord) {
    float minDistance = 0.12f;  // Sehr hoher Startwert für die minimale Distanz

    // Gehe alle Buckets durch und berechne den Abstand zu den gespeicherten Farben
    for (int i = 0; i < 8; i++) {
        float dist = colorDistance(color, imageLoad(kmeans, ivec2(i,0)).rgb);
        if (dist < minDistance) {
            return i;
        }
    }
    return -1;
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
    if (color.w == 1) {
        int bucketIndex = getSimilarBucketIndex(color.rgb, envCoord);

        if(bucketIndex == -1) {
            return;
        }

        ivec3 bucketPosition = ivec3(envCoord, bucketIndex);
        vec4 storedColor = imageLoad(buckets, bucketPosition);

        imageAtomicAdd(bucketCounters, bucketPosition, 1U);
        imageStore(buckets, bucketPosition, (storedColor + color));
    }
}
