#pragma once

#include <thread>
#include <future>
#include <type_traits>
#include <functional>
#include <exception>
#include <vector>
#include <atomic>

#include "thread_safe_queue.h"


class ThreadPoolExecutor {
public:
	explicit ThreadPoolExecutor(size_t thread_count);
	~ThreadPoolExecutor();


	template<typename Function, typename ...Args, typename ReturnType = std::invoke_result_t<Function&&, Args&&...>>
	std::future<ReturnType> submit(Function f, Args... args)
	{
		auto promise = std::make_shared<std::promise<ReturnType>>();
		auto future = promise->get_future();

		auto task = [func=std::move(f), ...lambda_args=std::move(args), promise]() mutable {
				if constexpr (std::is_same_v<ReturnType, void>) {
					func(lambda_args...);
					promise->set_value();
				} else {
					promise->set_value(func(lambda_args...));
				}
		};
		queue.push_front(std::move(task));
		return future;
	}

private:
	using task_type = std::function<void(void)>;

	ThreadSafeQueue<task_type> queue;
	std::vector<std::thread> threads;
	std::atomic<bool> should_continue;

	void stop();
};


inline ThreadPoolExecutor::ThreadPoolExecutor(size_t thread_count) :
	should_continue(true)
{
	for (size_t i = 0; i < thread_count; i++) {
		threads.emplace_back( [&]() {
			while (should_continue.load(std::memory_order::acquire) || !queue.empty()) {
				auto task = queue.pop_back();
				if (task) {
					(*task)();
				}
			}
		});
	}
}


inline ThreadPoolExecutor::~ThreadPoolExecutor()
{
	stop();
}

inline void ThreadPoolExecutor::stop()
{
	should_continue.store(false, std::memory_order::release);
	queue.set_do_not_wait_on_empty_queue(true);

	for (size_t i = 0; i < threads.size(); i++) {
		threads[i].join();
	}
}
