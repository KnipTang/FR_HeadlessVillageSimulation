#pragma once

#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#define NOMINMAX
#include <windows.h>
#include "../SimulationConfig.h"

// Class that represents a simple thread pool
namespace Rev
{
    class ThreadPool 
    {
    public:
        // // Constructor to creates a thread pool with given
        // number of threads
        ThreadPool(size_t num_threads
            = std::thread::hardware_concurrency())
        {
            // Creating worker threads
            for (size_t i = 0; i < num_threads; ++i) {
                m_Threads.emplace_back([this] {
                    while (true) {
                        std::function<void()> task;
                        // The reason for putting the below code
                        // here is to unlock the queue before
                        // executing the task so that other
                        // threads can perform enqueue tasks
                        {
                            // Locking the queue so that data
                            // can be shared safely
                            std::unique_lock<std::mutex> lock(
                                m_QueueMutex);

                            // Waiting until there is a task to
                            // execute or the pool is stopped
                            m_CV.wait(lock, [this] {
                                return !m_Tasks.empty() || m_Stop;
                                });
                            // exit the thread in case the pool
                            // is stopped and there are no tasks
                            if (m_Stop && m_Tasks.empty()) {
                                return;
                            }

                            // Get the next task from the queue
                            task = move(m_Tasks.front());
                            m_Tasks.pop();
                        }

                        task();
                    }
                });
            }
        }

        // Destructor to stop the thread pool
        ~ThreadPool()
        {
            {
                // Lock the queue to update the stop flag safely
                std::unique_lock<std::mutex> lock(m_QueueMutex);
                m_Stop = true;
            }

            // Notify all threads
            m_CV.notify_all();

            // Joining all worker threads to ensure they have
            // completed their tasks
            for (auto& thread : m_Threads) {
                thread.join();
            }
        }

        // Enqueue task for execution by the thread pool
        void Enqueue(std::function<void()> task)
        {
            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);
                m_Tasks.emplace(move(task));
            }
            m_CV.notify_one();
        }

        size_t ThreadCount() const { return m_Threads.size(); }

    private:
        // Vector to store worker threads
        std::vector<std::thread> m_Threads;

        // Queue of tasks
        std::queue<std::function<void()> > m_Tasks;

        // Mutex to synchronize access to shared data
        std::mutex m_QueueMutex;

        // Condition variable to signal changes in the state of
        // the tasks queue
        std::condition_variable m_CV;

        // Flag to indicate whether the thread pool should stop
        // or not
        bool m_Stop = false;
    };
}