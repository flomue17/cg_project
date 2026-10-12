#version 430
in vec2 tc;
out vec4 out_col;
uniform sampler2D diffuse;
void main() {
    out_col = texture(diffuse, tc);
//     out_col = vec4(tc, out_col.b, 1);
}
