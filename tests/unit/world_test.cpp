#include <gtest/gtest.h>

#include <map>

#include "engine/world/world_ca_generation.hpp"
#include "engine/world/world_cell.hpp"
#include "engine/particle/particle.hpp"

// TEST(World_CA_Generation_COUNT_SOLID, CountSolidNeigbors)
// {
//     World_CA_Generation world_gen;

//     WorldCell cell1(glm::ivec2(1, 1), create_stone());
//     WorldCell cell2(glm::ivec2(1, 1), create_stone());
//     WorldCell cell3(glm::ivec2(1, 1), create_stone());
//     WorldCell cell4(glm::ivec2(1, 1), create_stone());
//     WorldCell cell5();
//     WorldCell cell6();
//     WorldCell cell7();
//     WorldCell cell8();

//     EXPECT_EQ(8, world_gen.count_solid_neighbors(std::map<glm::ivec2, WorldCell *>{{glm::ivec2(1, 1), *cell1}}));
// }