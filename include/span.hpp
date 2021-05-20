/*
#Copyright 2021 xyang.
*/
#pragma once
#include <shared_mutex>
template<typename T>
struct RWSpan
{
    void Set(T v)
    {
        std::unique_lock lock(_stmutex);
        _vaule = v;
    }
    T& Get()
    {
        std::shared_lock lock(_stmutex);
        return _vaule;
    }
 private:
    std::shared_timed_mutex _stmutex;
    T _vaule;
};
