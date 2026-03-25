#pragma once

#include <memory>
#include "Scene/SceneManager.h"
#include "Grid/Grid.h"

namespace Rev
{
	class Rev_CoreSystems final
	{
	public:
		Rev_CoreSystems() = delete;
		~Rev_CoreSystems() = delete;

		//Core systems //Static vars can't depend on each other because static party problem
		//Needs inline to be able to declare static variable in header file
		static inline std::unique_ptr<Rev::SceneManager> pSceneManager = std::make_unique<Rev::SceneManager>();
	private:
	};
}