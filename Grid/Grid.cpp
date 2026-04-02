#include "Grid.h"
#include "../Rev_CoreSystems.h"
#include "../Scene/Scene.h"
#include "../GameObjects/Elements/BaseElement.h"
#include "../GameObjects/Elements/ElementManager.h"
#include "../GameObjects/Components/CompTransform.h"
#include "../GameObjects/Elements/HouseElement.h"
#include <iostream>
#include <stdlib.h>
#include "GridConsoleColors.h"

Grid::Grid(bool displayGrid) :
	m_CurrentTime{},
	m_UpdateGridTime{.2f},
	m_DisplayGrid{ displayGrid },
	m_UpdateGridRender{},
	m_UpdateGridRenderTime{0.1f},
	m_NonEmptyPositions{}
{
	m_ElementManager = dynamic_cast<ElementManager*>(AddChild(std::make_unique<ElementManager>()));

	m_TotalCells = g_gridHeight * g_gridWidth;
	const int avgCellLength = 12; // Approximate: color(5-8) + type(1-2) + reset(4) + space(1)
	m_DisplayBuffer.reserve(m_TotalCells * avgCellLength + g_gridHeight);
}

Grid::~Grid()
{

}

void Grid::Update(float deltaTime)
{
	m_CurrentTime += deltaTime;
	m_CurrentTimeRender += deltaTime;

	if (m_CurrentTime >= m_UpdateGridTime)
	{
		m_ElementManager->UpdateElements(m_CurrentTime);

		m_CurrentTime = 0;
		//{
		//	std::lock_guard<std::mutex> lock(m_GridMutex);
		//	m_ShouldRefresh = true;
		//}
		//m_CV.notify_one();
	}

	if (m_CurrentTimeRender >= m_UpdateGridRenderTime)
	{
		if (m_DisplayGrid)
			UpdateGridMap();
		m_UpdateGridRender = true;

		m_CurrentTimeRender = 0;
	}

	//GameObject::Update(deltaTime);
}

void Grid::Render()
{
	if (m_UpdateGridRender)
	{
		DisplayGrid();
		m_UpdateGridRender = false;
	}
}

void Grid::UpdateGridMap()
{
	for (Rev::Position& elemPos : m_NonEmptyPositions)
	{
		m_GridMap[elemPos.x + elemPos.y * g_gridWidth] = GridElement{};
	}
	m_NonEmptyPositions.clear();

	for (const auto& elem : m_ElementManager->GetElements())
	{
		if (!elem->IsActive())
			continue;

		Rev::Position elemPos = elem->transform->GetLocalPosition();
		if (elemPos.x >= 0 && elemPos.x < g_gridWidth &&
			elemPos.y >= 0 && elemPos.y < g_gridHeight)
		{
			m_NonEmptyPositions.emplace_back(elemPos);
			m_GridMap[elemPos.x + elemPos.y * g_gridWidth] = elem->GetGridElement();
		}
	}

	for (const auto& elem : m_ElementManager->GetHouseResources())
	{
		if (!elem->IsActive())
			continue;

		Rev::Position elemPos = elem->transform->GetLocalPosition();
		if (elemPos.x >= 0 && elemPos.x < g_gridWidth &&
			elemPos.y >= 0 && elemPos.y < g_gridHeight)
		{
			m_NonEmptyPositions.emplace_back(elemPos);
			m_GridMap[elemPos.x + elemPos.y * g_gridWidth] = elem->GetGridElement();
		}
	}
}

void Grid::DisplayGrid()
{
	if (m_DisplayGrid)
	{
		std::cout << "\033[2J\033[H";

		m_DisplayBuffer.clear();

		int rowEnd = g_gridWidth;
		for (int i = 0; i < m_TotalCells; i++)
		{
			const GridElement& element = m_GridMap[i];

			// Append color string
			m_DisplayBuffer.append(element.m_Color);

			// Convert int to string without allocation
			int num = element.m_TypeID;
			if (num == 0)
				m_DisplayBuffer += '0';
			else
			{
				char digits[4];
				int count = 0;
				while (num > 0)
				{
					digits[count++] = '0' + (num % 10);
					num /= 10;
				}
				while (count > 0)
					m_DisplayBuffer += digits[--count];
			}

			// Append reset and separator
			m_DisplayBuffer.append(RESET);
			m_DisplayBuffer += ((i + 1) == rowEnd) ? '\n' : ' ';

			if ((i + 1) == rowEnd)
				rowEnd += g_gridWidth;
		}

		std::cout << m_DisplayBuffer;
		std::cout << static_cast<int>(m_ElementManager->GetCycleState()) << '\n';
	}
}