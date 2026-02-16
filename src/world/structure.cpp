#include "engine/world/structure.hpp"

// Structure
Structure::Structure(const std::string &name, int width, int height) {}

void Structure::set_cell(int x, int y, const Particle &particle) {}

void Structure::set_cell(int x, int y, Particle_Type type, bool is_static) {}

void Structure::fill_rect(int x, int y, int w, int h, Particle_Type type, bool is_static) {}

const Particle &Structure::get_cell(int x, int y) const {}

bool Structure::in_bounds(int x, int y) const {}

int Structure::count_solid_cells() const {}

// StructureFactory
namespace StructureFactory
{
    Structure create_platform(int length, Particle_Type material) {}
}

// StructureSpawner
StructureSpawner::StructureSpawner() {}

void StructureSpawner::set_world(World *world) {}

void StructureSpawner::set_seed(int seed) {}

void StructureSpawner::add_blueprint(const std::string &name, const Structure &structure) {}

void StructureSpawner::add_blueprint(Structure &&structure) {}

Structure *StructureSpawner::get_blueprint(const std::string &name) {}

const std::map<std::string, Structure> &StructureSpawner::get_blueprints() const {}

void StructureSpawner::add_spawn_rule(const StructureSpawnRule &rule) {}

void StructureSpawner::clear_spawn_rules() {}

void StructureSpawner::setup_default_rules() {}

bool StructureSpawner::place_structure(const Structure &structure, const glm::ivec2 &world_pos) {}

bool StructureSpawner::place_structure_centered(const Structure &structure, const glm::ivec2 &center_pos) {}

bool StructureSpawner::check_min_distance(const glm::ivec2 &pos, const std::string &name,
                                          float min_dist_same, float min_dist_any) const {}

float StructureSpawner::check_empty_ratio(const Structure &structure, const glm::ivec2 &world_pos) const {}

bool StructureSpawner::check_chunks_exist(const Structure &structure, const glm::ivec2 &world_pos) const {}

int StructureSpawner::find_surface_y(int world_x, const glm::ivec2 &chunk_world_origin,
                                     int chunk_pixel_height) const {}

void StructureSpawner::try_spawn_in_chunk(const glm::ivec2 &chunk_coords, int chunk_width, int chunk_height) {}

void StructureSpawner::retry_pending_structures() {}

const std::vector<StructureSpawner::PlacedStructure> &StructureSpawner::get_placed_structures() const {}

// ImageStructureLoader
Structure ImageStructureLoader::load_from_image(const std::string &image_path) {}

std::map<std::string, Structure> ImageStructureLoader::load_all_from_folder(const std::string &folder_path) {}

Particle ImageStructureLoader::pixel_to_particle(int r, int g, int b, int a) {}

bool ImageStructureLoader::is_pixel_empty(int r, int g, int b, int a) {}

float ImageStructureLoader::color_distance(int r1, int g1, int b1, int r2, int g2, int b2) {}

std::array<int, 4> ImageStructureLoader::find_content_bounds(const unsigned char *data,
                                                             int width, int height, int channels) {}
