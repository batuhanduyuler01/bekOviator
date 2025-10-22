#include <deque>
#include <iostream>

template <typename T>
class FixedQueue {
public:
    FixedQueue(size_t capacity) : capacity_(capacity) {}

    void push(const T& item) {
        if (buffer_.size() == capacity_) {
            buffer_.pop_front();  // remove oldest
        }
        buffer_.push_back(item);
    }

    T pop() {
        if (buffer_.empty()) throw std::runtime_error("Queue empty");
        T item = buffer_.front();
        buffer_.pop_front();
        return item;
    }

    const T& front() const { return buffer_.front(); }
    const T& back() const { return buffer_.back(); }

    bool empty() const { return buffer_.empty(); }
    size_t size() const { return buffer_.size(); }
    std::deque<T> buffer_;
    size_t capacity_;
};