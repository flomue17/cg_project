#version 430 core

layout(binding = 0, rgba32f) uniform image2D u_mean;      // Environment Map (Eingabe und Ausgabe)
layout(binding = 1, r32ui) uniform uimage2D u_counter;         // Zähler der Pixel (Input)

layout(local_size_x = 1, local_size_y = 1) in;  // Workgroup-Größe

void main() {
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy);  // Bestimme den aktuellen Pixel

    // Lade die Summe der Farben und den Zählerwert
    vec4 colorSum = imageLoad(u_mean, coord);  // Die Summe der Farben an diesem Pixel (aus der Environment Map)
    uint counter = imageLoad(u_counter, coord).r;   // Der Zählerwert (Anzahl der validen Pixel)

    if(counter != 0) {
        imageStore(u_mean, coord, colorSum / float(counter));  // Mittelwert speichern
    }
}
