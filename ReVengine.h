#pragma once

#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace Rev
{
	class SceneManager;
}

namespace Rev
{
	class ReVengine final
	{
	public:
		ReVengine();
		~ReVengine();

		void Run(const std::function<SceneManager* ()>& GameRun);

	private:
		SceneManager* m_SceneMan;

	private:
		void RenderThreadFunction();

		std::thread m_RenderThread;
		std::mutex m_RenderMutex;
		std::condition_variable m_CV;
		std::atomic<bool> m_StopThread{ false };
		std::atomic<bool> m_ShouldRefresh{ false };
	};
}