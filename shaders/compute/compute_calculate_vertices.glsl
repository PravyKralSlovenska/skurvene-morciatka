#version 430 core

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

struct WorldCell {
    ivec2 coords;
    ivec2 _padding0;
    vec4 base_color;
    vec4 color;
    uvec4 meta; // x:type y:state z:move w:visited
};

struct ChunkInfo {
    ivec2 world_coords;
    int cell_data_offset;
    int cell_count;
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
uniform vec4 visible_bounds;

void main() {
    uint chunk_index = gl_GlobalInvocationID.z;
    if (chunk_index >= chunks.length()) return;

    ivec2 local_cell = ivec2(gl_GlobalInvocationID.xy);
    if (local_cell.x >= chunk_dimensions.x || local_cell.y >= chunk_dimensions.y) return;

    ChunkInfo chunk = chunks[chunk_index];
    
    int local_index = local_cell.y * chunk_dimensions.x + local_cell.x;
    int cell_index = chunk.cell_data_offset + local_index;

    WorldCell cell = cells[cell_index];

    // Filter empty
    if (cell.meta.x == 0u) return;

    // Calculate world position
    float world_x = float(chunk.world_coords.x * chunk_dimensions.x + local_cell.x) * particle_size;
    float world_y = float(chunk.world_coords.y * chunk_dimensions.y + local_cell.y) * particle_size;
    vec2 world_pos = vec2(world_x, world_y);

    // Frustum culling
    vec2 particle_max = world_pos + vec2(particle_size);
    if (particle_max.x < visible_bounds.x || world_pos.x > visible_bounds.z ||
        particle_max.y < visible_bounds.y || world_pos.y > visible_bounds.w) return;

    // Generate vertices
    uint vertex_base = atomicAdd(vertex_count, 6u);

    vec2 p0 = world_pos;
    vec2 p1 = world_pos + vec2(particle_size, 0.0);
    vec2 p2 = world_pos + vec2(0.0, particle_size);
    vec2 p3 = world_pos + vec2(particle_size);

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