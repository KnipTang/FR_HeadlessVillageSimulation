#pragma once
#include "../GameObjects/GameObject.h"
#include <memory>
#include <thread>
#include <mutex>

namespace Rev {
	class Scene;
}

class ElementManager;

class Grid : public Rev::GameObject
{
public:
	Grid();
	~Grid();

	Grid(const Grid& other) = default;
	Grid& operator=(const Grid& other) = default;
	Grid(Grid&& other) = default;
	Grid& operator=(Grid&& other) = default;

	virtual void Update(float deltaTime) override;

	//void AddToGrid(const BaseElement* element);

private:
	ElementManager* m_ElementManager;

	float m_CurrentTime;
	float m_DisplayRefreshTime;

private:
	void DisplayGrid();
	void DisplayThreadFunction();

	std::thread m_DisplayThread;
	std::mutex m_GridMutex;
	std::condition_variable m_CV;
	std::atomic<bool> m_StopThread{ false };
	std::atomic<bool> m_ShouldRefresh{ false };
};

