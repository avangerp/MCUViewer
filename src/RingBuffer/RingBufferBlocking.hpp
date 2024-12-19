#pragma once
#include <array>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>

using std::chrono::operator""ms;

template <typename T, size_t capacity>
class RingBufferBlocking
{
   public:
	explicit RingBufferBlocking() : read_idx(0), write_idx(0), size_(0) {}

	bool push(const T& item)
	{
		std::unique_lock<std::mutex> lock(mutex);
		if (!cond_full.wait_for(lock, 100ms, [this]()
								{ return size_ < capacity; }))
			return false;

		buffer[write_idx] = item;
		write_idx = (write_idx + 1) % capacity;
		size_++;

		cond_empty.notify_one();
		return true;
	}

	T pop()
	{
		std::unique_lock<std::mutex> lock(mutex);
		cond_empty.wait(lock, [this]()
						{ return size_ > 0; });

		T item = buffer[read_idx];
		read_idx = (read_idx + 1) % capacity;
		size_--;

		cond_full.notify_one();
		return item;
	}

	size_t size()
	{
		std::unique_lock<std::mutex> lock(mutex);
		return size_;
	}

	void clear()
	{
		while (size_)
			pop();
	}

   private:
	std::array<T, capacity> buffer;
	size_t read_idx;
	size_t write_idx;
	size_t size_;

	std::mutex mutex;
	std::condition_variable cond_empty;
	std::condition_variable cond_full;
};
