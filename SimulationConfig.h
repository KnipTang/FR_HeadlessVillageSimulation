#pragma once

inline constexpr int g_AgentsCount = 50;
inline constexpr int g_FiniteResourcesCount = 20;
inline constexpr int g_HousesCount = 15;

//In seconds
inline constexpr int g_DayTime = 50;
inline constexpr int g_NightTime = 5;

inline constexpr unsigned char g_AgentID = 1;
inline constexpr unsigned char g_FiniteResourceID = 2;
inline constexpr unsigned char g_HousesID = 3;

inline constexpr unsigned char g_HouseCapacity = 3;

#include <chrono>

static void burnCPU(double seconds) {
	auto start = std::chrono::steady_clock::now();

	// Smaller, configurable workload
	const int iterations = 1000000; // Reduced from 100M to 1M

	while (true) {
		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();

		if (elapsed >= seconds * 1000) break;

		// Smaller floating-point operations
		volatile double result = 0.0;
		for (volatile int i = 0; i < iterations; ++i) {
			result += i * 3.14159;
			result -= i / 2.71828;
		}
	}
}