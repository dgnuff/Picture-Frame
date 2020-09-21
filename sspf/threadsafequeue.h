#pragma once

template<typename T>
class threadSafeQueue
{
public:
    threadSafeQueue()
    {
    }

    virtual ~threadSafeQueue()
    {
    }

    void    enqueue(T const &payload)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(payload);
    }

    bool    get(T &payload)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        bool const result = !_queue.empty();
        if (result)
        {
            payload = _queue.front();
            _queue.pop();
        }
        return result;
    }

    int size()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return (int) _queue.size();
    }

private:
    std::queue<T>	_queue;
    std::mutex      _mutex;
};
