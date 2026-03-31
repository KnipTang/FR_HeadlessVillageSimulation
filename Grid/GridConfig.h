#pragma once
#include <random>

inline constexpr int g_gridWidth = 20;
inline constexpr int g_gridHeight = 20;

static std::random_device rd;
static std::default_random_engine dre(rd());
static std::uniform_int_distribution<int> uniform_dist_width(0, g_gridWidth - 1);
static std::uniform_int_distribution<int> uniform_dist_height(0, g_gridHeight - 1);