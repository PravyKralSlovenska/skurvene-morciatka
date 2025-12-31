#version 430 core

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

struct Particle {
    uint type;
    uint state;
    uint _pad1;
    uint _pad2;
    vec4 color;
};

struct Vertex {
    vec2 position;
    vec4 color;
};

layout(std430, binding = 0) readonly buffer ParticleBuffer {
    Particle particles[];
};

layout(std430, binding = 1) writeonly buffer VertexBuffer {
    Vertex vertices[];
};

uniform vec2 chunk_world_offset;
uniform float particle_size;
uniform ivec2 chunk_dimensions;

void main() {
    ivec2 cell_coords = ivec2(gl_GlobalInvocationID.xy);
    
    if (cell_coords.x >= chunk_dimensions.x || 
        cell_coords.y >= chunk_dimensions.y) {
        return;
    }
    
    int particle_index = cell_coords.y * chunk_dimensions.x + cell_coords.x;
    
    Particle particle = particles[particle_index];
    
    if (particle.type == 0) {
        return;
    }
    
    vec2 world_pos = chunk_world_offset + vec2(cell_coords) * particle_size;
    
    int vertex_base = particle_index * 4;
    
    vertices[vertex_base + 0] = Vertex(
        world_pos,
        particle.color
    );
    
    vertices[vertex_base + 1] = Vertex(
        world_pos + vec2(particle_size, 0.0),
        particle.color
    );
    
    vertices[vertex_base + 2] = Vertex(
        world_pos + vec2(0.0, particle_size),
        particle.color
    );
    
    vertices[vertex_base + 3] = Vertex(
        world_pos + vec2(particle_size, particle_size),
        particle.color
    );
}
