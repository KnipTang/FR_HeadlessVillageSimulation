#pragma once
#include <thread>

inline static const unsigned char g_TotalThreadCount = std::thread::hardware_concurrency();
inline static const unsigned char g_AgentThreadCount = g_TotalThreadCount / 4;
inline static const unsigned char g_ResourceThreadCount = g_TotalThreadCount / 4;