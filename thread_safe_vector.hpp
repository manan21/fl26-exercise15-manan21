#ifndef THREAD_SAFE_VECTOR_HPP
#define THREAD_SAFE_VECTOR_HPP

#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

template<typename T>
class ThreadSafeVector {
public:
    ThreadSafeVector() = default;
    ThreadSafeVector(const ThreadSafeVector&) = delete;
    ThreadSafeVector& operator=(const ThreadSafeVector&) = delete;

    void push_back(const T& value)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            values_.push_back(value);
        }
        condition_.notify_one();
    }

    void push_back(T&& value)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            values_.push_back(std::move(value));
        }
        condition_.notify_one();
    }

    void wait_and_pop_back(T& value)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] { return !values_.empty(); });
        value = values_.back();
        values_.pop_back();
    }

    std::shared_ptr<T> wait_and_pop_back()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] { return !values_.empty(); });
        std::shared_ptr<T> value = std::make_shared<T>(values_.back());
        values_.pop_back();
        return value;
    }

    bool try_pop_back(T& value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (values_.empty()) {
            return false;
        }
        value = values_.back();
        values_.pop_back();
        return true;
    }

    std::shared_ptr<T> try_pop_back()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (values_.empty()) {
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> value = std::make_shared<T>(values_.back());
        values_.pop_back();
        return value;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return values_.empty();
    }

    std::size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return values_.size();
    }

private:
    std::vector<T> values_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
};

#endif
