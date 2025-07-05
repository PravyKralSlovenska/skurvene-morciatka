#pragma once
#include <string>

std::string read_file(const std::string &filepath);

unsigned int compile_shader(unsigned int type, const std::string &source);
unsigned int create_shader(const std::string &vertex_shader_path, const std::string &fragment_shader_path);

/*
 * v utils.hpp
 * - pomocne funkcia na zistenie ci je WorldCell/Particle validny v mriezke sveta
 */
bool in_world_range(int x, int y, int world_rows, int world_cols);