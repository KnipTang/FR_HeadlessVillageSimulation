#include "Grid.h"
#include "../Rev_CoreSystems.h"
#include "../Scene/Scene.h"
#include "../GameObjects/Elements/BaseElement.h"
#include "../GameObjects/Elements/ElementManager.h"
#include "../GameObjects/Components/CompTransform.h"
#include <iostream>
#include <stdlib.h>
#include "GridConsoleColors.h"

Grid::Grid() :
	m_CurrentTime{},
	m_DisplayRefreshTime{1.f}
{
	m_ElementManager = dynamic_cast<ElementManager*>(AddChild(std::make_unique<ElementManager>()));

	m_DisplayThread = std::thread(&Grid::DisplayThreadFunction, this);
}

Grid::~Grid()
{
	{
		std::lock_guard<std::mutex> lock(m_GridMutex);
		m_StopThread = true;
	}
	m_CV.notify_one();

	if (m_DisplayThread.joinable())
	{
		m_DisplayThread.join();
	}
}

void Grid::Update(float deltaTime)
{
	m_CurrentTime += deltaTime;
	if (m_CurrentTime >= m_DisplayRefreshTime)
	{
		m_CurrentTime = 0;

		{
			std::lock_guard<std::mutex> lock(m_GridMutex);
			m_ShouldRefresh = true;
		}
		m_CV.notify_one();
	}

	GameObject::Update(deltaTime);
}

void Grid::DisplayGrid()
{
	GridElement m_GridMap[g_gridWith * g_gridHeight]{};

	for (const auto& elem : m_ElementManager->GetElements())
	{
		if (!elem->IsActive())
			continue;

		Rev::Position elemPos = elem->transform->GetLocalPosition();
		if (elemPos.x >= 0 && elemPos.x < g_gridWith &&
			elemPos.y >= 0 && elemPos.y < g_gridHeight)
		{
			m_GridMap[elemPos.x + elemPos.y * g_gridWith] = elem->GetGridElement();
		}
	}

	system("cls");

	for (int y = 0; y < g_gridHeight; y++)
	{
		for (int x = 0; x < g_gridWith; x++)
		{
			GridElement gridElement = m_GridMap[x + y * g_gridWith];
			std::cout << gridElement.m_Color << gridElement.m_TypeID << RESET << " ";
		}
		std::cout << '\n';
	}
}

void Grid::DisplayThreadFunction()
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(m_GridMutex);

		m_CV.wait(lock, [this] {
			return m_ShouldRefresh.load() || m_StopThread.load();
			});

		if (m_StopThread)
		{
			break;
		}

		m_ShouldRefresh = false;

		DisplayGrid();
	}

}
