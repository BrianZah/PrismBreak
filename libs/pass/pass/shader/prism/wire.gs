#version 450 core
layout (triangles) in;
layout (line_strip, max_vertices = 3) out;

void main() {
    gl_Position = gl_in[0].gl_Position-vec4(0.0, 0.0, 0.0001, 0.0);
    EmitVertex();

    gl_Position = gl_in[1].gl_Position-vec4(0.0, 0.0, 0.0001, 0.0);
    EmitVertex();

    gl_Position = gl_in[2].gl_Position-vec4(0.0, 0.0, 0.0001, 0.0);
    EmitVertex();

    EndPrimitive();
}
