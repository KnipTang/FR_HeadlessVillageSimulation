#include "ReVengine.h"
#include <chrono>
#include <thread>
#include "Scene/SceneManager.h"
#include <iostream>
#include <windows.h>
#include "SimulationConfig.h"

#define MS_PER_FRAME(fps) (1000.0f / (fps))
constexpr int fps = 120;

using namespace Rev;

ReVengine::ReVengine()
{
	std::srand(static_cast<unsigned>(std::time(nullptr)));
}

ReVengine::~ReVengine()
{

}

void ReVengine::Run(const std::function<SceneManager*()>& GameRun)
{
	SceneManager* sceneMan = GameRun();

	sceneMan->Init();

	auto lastTime = std::chrono::high_resolution_clock::now();
	float lag = 0.0f;

	const float fixedTimeStep = 1.0f / fps;

	constexpr auto targetFrameTime = std::chrono::milliseconds(static_cast<long long>(MS_PER_FRAME(fps)));

	int frameCount = 0;
	float fpsTimer = 0.0f;
	float currentFPS = 0.0f;

	bool quit = false;
	while (quit == false)
	{
		const auto currentTime = std::chrono::high_resolution_clock::now();
		const float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;
		lag += deltaTime;

		while (lag >= fixedTimeStep)
		{
			sceneMan->FixedUpdate(fixedTimeStep);

			lag -= fixedTimeStep;
		}

		sceneMan->Update(deltaTime);

		sceneMan->LateUpdate(deltaTime);

		{
			// FPS calculation
			frameCount++;
			fpsTimer += deltaTime;

			if (fpsTimer >= 1.0f) // Update FPS display every second
			{
				currentFPS = frameCount / fpsTimer;

				char buffer[256];
				sprintf_s(buffer, "FPS: %d | Target: %d\n", static_cast<int>(currentFPS), fps);
				OutputDebugStringA(buffer);  // View in Visual Studio Output window or DebugView


				// Reset counters
				frameCount = 0;
				fpsTimer = 0.0f;
			}
		}

		burnCPU(0.03f);

		const auto sleepTime = currentTime + targetFrameTime - std::chrono::high_resolution_clock::now();
		std::this_thread::sleep_for(sleepTime);
	}
}