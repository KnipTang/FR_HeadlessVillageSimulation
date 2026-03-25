#include "ReVengine.h"
#include "Rev_CoreSystems.h"
#include "GameObjects/GameObject.h"
#include "GameObjects/Components/CompTransform.h"
#include "GameObjects/Elements/AgentElement.h"
#include "GameObjects/Elements/ElementManager.h"
#include "GameObjects/Elements/ResourceElement.h"
#include "GameObjects/Elements/HouseElement.h"
#include "Grid/Grid.h"
#include "Scene/Scene.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

std::unique_ptr<Rev::Scene> Scene1()
{
	std::unique_ptr<Grid> grid = std::make_unique<Grid>();

	//Scene add gameobects & return
	{
		std::unique_ptr<Rev::Scene> scene = std::make_unique<Rev::Scene>();
		scene->SetActive(true);

		Grid* testG = scene->AddGameObject(std::move(grid));

		//scene->DisplaySceneHierarchy();

		return std::move(scene);
	}
}

Rev::SceneManager* Load()
{
	std::unique_ptr<Rev::Scene> scene(Scene1());

	Rev::Rev_CoreSystems::pSceneManager->AddScene(std::move(scene));
	return Rev::Rev_CoreSystems::pSceneManager.get();
}

int main(int argc, char* argv[])
{
	std::unique_ptr<Rev::ReVengine> pReVengine;
	pReVengine = std::make_unique<Rev::ReVengine>();
	
	pReVengine->Run(Load);

	return 0;
}