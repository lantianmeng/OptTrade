#pragma once
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <future>
#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include "threadsafe_queue.h"

class threadpool
{
public:
	threadpool(unsigned short thread_nums = std::thread::hardware_concurrency()):stoped(false)
	{
		start(thread_nums);
	}

	~threadpool()
	{
		stop();
	}

	// 提交一个任务
	// 调用.get()获取返回值会等待任务执行完,获取返回值
	// 有两种方法可以实现调用类成员，
	// 一种是使用   bind： .commit(std::bind(&Dog::sayHello, &dog));
	// 一种是用 mem_fn： .commit(std::mem_fn(&Dog::sayHello), &dog)
	template<class F, class... Args>
	auto commit(F&& f, Args&&... args) ->std::future<decltype(f(args...))>
	{
		if (stoped.load())    // stop == true ??
			throw std::runtime_error("commit on ThreadPool is stopped.");

		using RetType = decltype(f(args...)); // typename std::result_of<F(Args...)>::type, 函数 f 的返回值类型
		auto task = std::make_shared<std::packaged_task<RetType()> >(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);    // wtf !
		std::future<RetType> future = task->get_future();
		tasks.push([task](){ (*task)();});
		return future;
	}
private:
	void start(unsigned short thread_nums)
	{
		for (int i = 0; i < thread_nums; ++i)
		{
			pool.emplace_back(std::thread(&threadpool::RunInThread, this));
		}
	}

	void RunInThread()
	{
		Task task;
		while (!stoped)
		{
			tasks.wait_and_pop(task);
			task();
		}
	}

	void stop()
	{
		stoped.store(true);

		for (auto &thread : pool)
			if (thread.joinable()) thread.join();

	}
private:
	using Task = std::function<void()>;
	std::vector<std::thread> pool;
	std::atomic_bool stoped;
	threadsafe_queue<Task> tasks;

};

#endif // !THREADPOOL_H
