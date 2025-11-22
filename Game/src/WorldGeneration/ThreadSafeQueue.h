#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>
#include <atomic>

// Provides a minimal thread-safe queue for cross-thread handoff of chunk jobs and results.
template <typename T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue() = default;

    // Push a copy/move of the provided item and wake one waiting consumer.
    void Push(T value)
    {
        std::lock_guard<std::mutex> l_Lock(m_Mutex);
        m_Queue.push(std::move(value));
        m_Condition.notify_one();
    }

    // Attempt to pop immediately without blocking.
    bool TryPop(T& outValue)
    {
        std::lock_guard<std::mutex> l_Lock(m_Mutex);
        if (m_Queue.empty())
        {
            return false;
        }

        outValue = std::move(m_Queue.front());
        m_Queue.pop();

        return true;
    }

    // Block until an item is available or a stop flag triggers.
    bool WaitPop(T& outValue, const std::atomic<bool>& stopFlag)
    {
        std::unique_lock<std::mutex> l_Lock(m_Mutex);
        m_Condition.wait(l_Lock, [&]() { return stopFlag.load() || !m_Queue.empty(); });

        if (stopFlag.load())
        {
            return false;
        }

        outValue = std::move(m_Queue.front());
        m_Queue.pop();

        return true;
    }

    // Wake all waiting threads (used when shutting down the queue owner).
    void NotifyAll()
    {
        m_Condition.notify_all();
    }

private:
    std::queue<T> m_Queue;
    mutable std::mutex m_Mutex;
    std::condition_variable m_Condition;
};