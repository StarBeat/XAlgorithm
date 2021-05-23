/*
#Copyright 2021 xyang.
*/

#pragma once
#include <atomic>
#include <thread> // NOLINT
#include <type_traits>
#include <vector>
#include <queue>
#include <cassert>

namespace x::xalgorithm::lockfree
{
using std::queue;
template<typename T>
concept WhereTisTrivial = requires()
{
    ::std::is_trivial_v<T>;
};

template<WhereTisTrivial T>
struct CircularCasQueue
{
 public:
    enum Strategy : char
    {
        Enm_ABANDON,
        Enm_FORCE,
        Enm_YIELD,
    };

    explicit CircularCasQueue(int capacity = 1024) : _capacity(capacity)
    {
        _data.reserve(capacity);
    }

    ~CircularCasQueue() = default;

    bool full()
    {
        return _size.load() == _capacity;
    }

    bool empty()
    {
        return _size.load() == 0;
    }

    size_t size()
    {
        return _size.load();
    }

    bool push(const T& in_value, Strategy strategy = Strategy::Enm_FORCE)
    {
        int rear = _rear.load();
        while (true)
        {
            if (rear == -1 || full())
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
                if (full())
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

    bool pop(const T& out_value, Strategy strategy = Strategy::Enm_FORCE)
    {
        int front = _front.load();
        while (true)
        {
            if (front == -1 || empty())
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
                if (empty())
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
#pragma region concept for std

    void push(T&& v)
    {
        push(std::move(v), Strategy::Enm_FORCE);
    }

    T& pop()
    {
        T* ret;
        if (pop(*ret, Strategy::Enm_FORCE))
        {
            return *ret;
        }
        return *reinterpret_cast<T*>(nullptr);
    }

#pragma endregion
 private:
    const int _capacity;
    std::vector<T> _data;
    std::atomic<int> _size;
    std::atomic<int> _front;
    std::atomic<int> _rear;
};

template<typename T>
class AtomLockQueue : protected queue<T>
{
 public:
    AtomLockQueue() : queue<T>::queue()
    {
    }

    ~AtomLockQueue()
    {
        queue<T>::~queue();
    }

    void push(const T& v)
    {
        lock_guard l(_busy);
        queue<T>::push(v);
    }

    void push(T&& v)
    {
        lock_guard l(_busy);
        queue<T>::push(std::forward<T>(v));
    }

    template <class... Args>
    void emplace(Args&&... args)
    {
        lock_guard l(_busy);
        queue<T>::emplace(std::forward<Args>(args)...);
    }

    size_t size()
    {
        lock_guard l(_busy);
        return queue<T>::size();
    }

    bool empty()
    {
        lock_guard l(_busy);
        return queue<T>::empty();
    }

    void swap(queue<T>& _Right)
    {
        lock_guard l(_busy);
        queue<T>::swap(_Right); // NOLINT
    }

    T& pop()
    {
        lock_guard l(_busy);
        if (queue<T>::empty())
        {
            return *reinterpret_cast<T*>(nullptr);
        }
        auto rt = queue<T>::front();
        queue<T>::pop();
        return rt;
    }

    T& front()
    {
        lock_guard l(_busy);
        if (queue<T>::empty())
        {
            return *reinterpret_cast<T*>(nullptr);
        }
        return queue<T>::front();
    }
    T& back()
    {
        lock_guard l(_busy);
        if (queue<T>::empty())
        {
            return *reinterpret_cast<T*>(nullptr);
        }
        return queue<T>::back();
    }
 private:
    struct lock_guard
    {
        explicit lock_guard(std::atomic<bool>& a) : __busy(a)
        {
            bool expect = false;
            while (!__busy.compare_exchange_weak(expect, true))
            {
                expect = false;
            }
        }
        ~lock_guard()
        {
            __busy.store(false);
        }
        std::atomic<bool>& __busy;
    };
    std::atomic<bool> _busy = false;
};
}  // namespace x::xalgorithm::lockfree
