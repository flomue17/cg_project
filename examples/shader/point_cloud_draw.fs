#version 330 core

in vec3 col;

layout (location = 0) out vec4 out_col;

void main()
{
    out_col = vec4(col, 1);
}
