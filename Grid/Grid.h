#pragma once
#include "../GameObjects/GameObject.h"
#include "GridConsoleColors.h"
#include "GridConfig.h"
#include <memory>
#include <thread>
#include <mutex>
#include <vector>

namespace Rev {
	class Scene;
	struct Position;
}

class ElementManager;

class ThreadPool;

class Grid : public Rev::GameObject
{
public:
	Grid(bool displayGrid = true);
	~Grid();

	Grid(const Grid& other) = default;
	Grid& operator=(const Grid& other) = default;
	Grid(Grid&& other) = default;
	Grid& operator=(Grid&& other) = default;

	virtual void Update(float deltaTime) override;
	virtual void Render() override;

	//void AddToGrid(const BaseElement* element);

private:
	ElementManager* m_ElementManager;

	float m_CurrentTime;
	float m_UpdateGridTime;

	bool m_DisplayGrid;

	bool m_UpdateGridRender;
	float m_CurrentTimeRender;
	float m_UpdateGridRenderTime;

	GridElement m_GridMap[g_gridWidth * g_gridHeight]{};
	std::vector<Rev::Position> m_NonEmptyPositions;

private:
	void DisplayGrid();
};

