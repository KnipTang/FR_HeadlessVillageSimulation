#pragma once

#include <functional>
#include <memory>

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
	};
}