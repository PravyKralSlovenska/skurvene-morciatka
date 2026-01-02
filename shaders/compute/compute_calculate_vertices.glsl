#version 430 core

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

struct WorldCell {
    vec4 base_color;
    vec4 color;
    vec2 world_coords;
    uvec2 meta; // meta.x = particle_type
};

struct ChunkInfo {
    ivec2 coords;
    ivec2 padding;
};

struct Vertex {
    vec2 position;
    vec2 _padding;
    vec4 color;
};

layout(std430, binding = 0) readonly buffer WorldCellBuffer {
    WorldCell cells[];
};

layout(std430, binding = 1) readonly buffer ChunkBuffer {
    ChunkInfo chunks[];
};

layout(std430, binding = 2) writeonly buffer VertexBuffer {
    Vertex vertices[];
};

layout(std430, binding = 3) buffer VertexCounter {
    uint vertex_count;
};

uniform float particle_size;
uniform ivec2 chunk_dimensions;
uniform int cells_per_chunk;
uniform int chunk_count;
uniform vec4 visible_bounds; // x=min_x, y=min_y, z=max_x, w=max_y

void main() {
    uint chunk_index = gl_GlobalInvocationID.z;
    if (chunk_index >= uint(chunk_count)) {
        return;
    }

    ivec2 cell_coords = ivec2(gl_GlobalInvocationID.xy);
    if (cell_coords.x >= chunk_dimensions.x ||
        cell_coords.y >= chunk_dimensions.y) {
        return;
    }

    uint local_index = uint(cell_coords.y * chunk_dimensions.x + cell_coords.x);
    uint cell_index = chunk_index * uint(cells_per_chunk) + local_index;

    WorldCell cell = cells[cell_index];
    
    // GPU-side empty particle filtering - just skip, don't write
    if (cell.meta.x == 0u) {
        return;
    }

    // Per-particle frustum culling
    vec2 particle_max = cell.world_coords + vec2(particle_size, particle_size);
    if (particle_max.x < visible_bounds.x || cell.world_coords.x > visible_bounds.z ||
        particle_max.y < visible_bounds.y || cell.world_coords.y > visible_bounds.w) {
        return;
    }

    // Atomically allocate space for 6 vertices
    uint vertex_base = atomicAdd(vertex_count, 6u);

    // Use pre-calculated world coordinates
    vec2 world_pos = cell.world_coords;

    vec2 p0 = world_pos;
    vec2 p1 = world_pos + vec2(particle_size, 0.0);
    vec2 p2 = world_pos + vec2(0.0, particle_size);
    vec2 p3 = world_pos + vec2(particle_size, particle_size);

    vertices[vertex_base + 0].position = p0;
    vertices[vertex_base + 0].color = cell.color;

    vertices[vertex_base + 1].position = p1;
    vertices[vertex_base + 1].color = cell.color;

    vertices[vertex_base + 2].position = p2;
    vertices[vertex_base + 2].color = cell.color;

    vertices[vertex_base + 3].position = p1;
    vertices[vertex_base + 3].color = cell.color;

    vertices[vertex_base + 4].position = p3;
    vertices[vertex_base + 4].color = cell.color;

    vertices[vertex_base + 5].position = p2;
    vertices[vertex_base + 5].color = cell.color;
}