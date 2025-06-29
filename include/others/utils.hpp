#pragma once
#include <string>

std::string read_file(const std::string &filepath);

unsigned int compile_shader(unsigned int type, const std::string &source);
unsigned int create_shader(const std::string &vertex_shader_path, const std::string &fragment_shader_path);