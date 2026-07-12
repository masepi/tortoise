#pragma once

#include <mutex>
#include <deque>
#include <condition_variable>
#include <optional>
#include <utility>


template<typename T>
class ThreadSafeQueue {
public:
	ThreadSafeQueue() : do_not_wait_on_empty_queue(false) {}

	using value_type = T;
	using size_type = typename std::deque<T>::size_type;
	
	void push_back(T&& value);
	void push_front(T&& value);
	std::optional<T> pop_back();
	std::optional<T> pop_front();

	void set_do_not_wait_on_empty_queue(bool value);

	bool empty();
	size_type size();
	size_type clear();

private:
	std::mutex m;
	bool do_not_wait_on_empty_queue;
	std::condition_variable cv;
	std::deque<T> queue;
};

template <typename T>
inline void ThreadSafeQueue<T>::push_back(T&& value)
{
	{
		std::lock_guard lock{m};
		queue.push_back(std::forward<T>(value));
	}
	cv.notify_one();
}

template <typename T>
inline void ThreadSafeQueue<T>::push_front(T&& value)
{
	{
		std::lock_guard lock{m};
		queue.push_front(std::forward<T>(value));
	}
	cv.notify_one();
}

template <typename T>
inline std::optional<T> ThreadSafeQueue<T>::pop_back()
{
	std::unique_lock lock{m};

	if (queue.empty()) {
		if (do_not_wait_on_empty_queue) {
			return std::nullopt;
		}

		cv.wait(lock, [&]() { return !queue.empty() || do_not_wait_on_empty_queue; });
		if (queue.empty()) {
			return std::nullopt;
		}
	}

	T elem = std::move(queue.back());
	queue.pop_back();
	return elem;
}

template <typename T>
inline std::optional<T> ThreadSafeQueue<T>::pop_front()
{
	std::unique_lock lock{m};

	if (queue.empty()) {
		if (do_not_wait_on_empty_queue) {
			return std::nullopt;
		}

		cv.wait(lock, [&]() { return !queue.empty() || do_not_wait_on_empty_queue; });
		if (queue.empty()) {
			return std::nullopt;
		}
	}


	T elem = std::move(queue.front());
	queue.pop_front();
	return elem;
}

template <typename T>
inline void ThreadSafeQueue<T>::set_do_not_wait_on_empty_queue(bool value)
{
	{
		std::lock_guard lock{m};
		do_not_wait_on_empty_queue = value;
	}

	cv.notify_all();
}

template <typename T>
inline bool ThreadSafeQueue<T>::empty()
{
	std::lock_guard lock{m};
	return queue.empty();
}

template <typename T>
inline ThreadSafeQueue<T>::size_type ThreadSafeQueue<T>::size()
{
	std::lock_guard lock{m};
	return queue.size();
}

template <typename T>
inline ThreadSafeQueue<T>::size_type ThreadSafeQueue<T>::clear()
{
	std::lock_guard lock{m};
	const size_type size = queue.size();
	queue.clear();
	return size;
}
