/*
#Copyright 2021 xyang.
*/

#pragma once
#include <queue>
#include <shared_mutex>

namespace X::XAlgorithm
{
template<typename T>
struct Queue
{
 private:
    std::queue<T> _queue;
};
template<typename T>
struct RWSpan
{
    void set(T v)
    {
        std::unique_lock lock(_stmutex);
    }
    T& get()
    {
        std::shared_lock lock(_stmutex);
        return vaule;
    }
 private:
    std::shared_timed_mutex _stmutex;
    T vaule;
};
}  // namespace X::XAlgorithm
