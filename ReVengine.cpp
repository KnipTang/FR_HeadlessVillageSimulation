#include "ReVengine.h"
#include <chrono>
#include <thread>
#include "Scene/SceneManager.h"
#include <iostream>
#define NOMINMAX
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
	{
		std::lock_guard<std::mutex> lock(m_RenderMutex);
		m_StopThread = true;
	}
	m_CV.notify_one();

	if (m_RenderThread.joinable())
	{
		m_RenderThread.join();
	}
}

void ReVengine::Run(const std::function<SceneManager*()>& GameRun)
{
	m_SceneMan = GameRun();

	m_RenderThread = std::thread(&ReVengine::RenderThreadFunction, this);

	m_SceneMan->Init();

	auto lastTime = std::chrono::high_resolution_clock::now();
	//float lag = 0.0f;

//	const float fixedTimeStep = 1.0f / fps;

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
		//lag += deltaTime;

		//while (lag >= fixedTimeStep)
		//
			//m_SceneMan->FixedUpdate(fixedTimeStep);

//lag -= fixedTimeStep;
		//}

		m_SceneMan->Update(deltaTime);

		//m_SceneMan->LateUpdate(deltaTime);

		{
			std::lock_guard<std::mutex> lock(m_RenderMutex);
			m_ShouldRefresh = true;
		}
		m_CV.notify_one();

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

		const auto sleepTime = currentTime + targetFrameTime - std::chrono::high_resolution_clock::now();
		std::this_thread::sleep_for(sleepTime);
	}
}

void Rev::ReVengine::RenderThreadFunction()
{
	while (m_SceneMan->IsRenderingEnabled())
	{
		std::unique_lock<std::mutex> lock(m_RenderMutex);

		m_CV.wait(lock, [this] {
			return m_ShouldRefresh.load() || m_StopThread.load();
			});

		if (m_StopThread)
		{
			break;
		}

		m_ShouldRefresh = false;

		m_SceneMan->Render();
	}
}
