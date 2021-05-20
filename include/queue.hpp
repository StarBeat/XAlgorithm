/*
#Copyright 2021 xyang.
*/

#pragma once
#include <atomic>
#include <thread> // NOLINT
#include <type_traits>
#include <vector>

namespace x::xalgorithm
{
template<typename T>
concept WhereTisTrivial = requires()
{
    ::std::is_trivial_v<T>;
};

template<WhereTisTrivial T>
struct Queue
{
 public:
    enum Strategy : char
    {
        Enm_ABANDON,
        Enm_FORCE,
        Enm_YIELD,
    };

    explicit Queue(int capacity) : _capacity(capacity)
    {
        _data.reserve(capacity);
    }

    ~Queue() = default;

    bool IsFull()
    {
        return _size.load() == _capacity;
    }

    bool IsEmpty()
    {
        return _size.load() == 0;
    }

    bool Push(const T& in_value, Strategy strategy = Strategy::Enm_FORCE)
    {
        int rear = _rear.load();
        while (true)
        {
            if (rear == -1 || IsFull())
            {
                switch (strategy)
                {
                case Strategy::Enm_YIELD:
                    std::this_thread::yield();
                case Strategy::Enm_FORCE:
                    rear = _rear.load();
                    continue;
                }
                return false;
            }
            if (_rear.compare_exchange_weak(rear, -1))
            {
                if (IsFull())
                {
                    int excepted = -1;
                    bool flag = _rear.compare_exchange_weak(excepted, rear);
                    assert(flag);
                    continue;
                }
                break;
            }
        }
        _data[rear] = in_value;
        ++_size;
        int excepted = -1;
        bool flag = _rear.compare_exchange_weak(excepted, (rear + 1) % _capacity);
        assert(flag);
        return true;
    }

    bool Pop(const T& out_value, Strategy strategy = Strategy::Enm_FORCE)
    {
        int front = _front.load();
        while (true)
        {
            if (front == -1 || IsEmpty())
            {
                switch (strategy)
                {
                case Strategy::Enm_YIELD:
                    std::this_thread::yield();
                case Strategy::Enm_FORCE:
                    front = _front.load();
                    continue;
                }
                return false;
            }
            if (_front.compare_exchange_weak(front, -1))
            {
                if (IsEmpty())
                {
                    int excepted = -1;
                    bool flag = _front.compare_exchange_weak(excepted, front);
                    assert(flag);
                    continue;
                }
                break;
            }
        }
        out_value = _data[front];
        --_size;
        int excepted = -1;
        bool flag = _front.compare_exchange_weak(excepted, (front + 1) % _capacity);
        assert(flag);
        return true;
    }
 private:
    const int _capacity;
    std::vector<T> _data;
    std::atomic<int>_size;
    std::atomic<int>_front;
    std::atomic<int>_rear;
};

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
}  // namespace x::xalgorithm
