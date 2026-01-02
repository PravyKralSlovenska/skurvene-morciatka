#version 430 core

struct Vertex {
    vec2 position;
    vec2 _padding;
    vec4 color;
};

layout(std430, binding = 0) readonly buffer Vertices {
    Vertex vertices[];
};

uniform mat4 view_projection;
out vec4 vColor;

void main() {
    Vertex v = vertices[gl_VertexID];

    gl_Position = view_projection * vec4(v.position, 0.0, 1.0);

    vColor = v.color;
}