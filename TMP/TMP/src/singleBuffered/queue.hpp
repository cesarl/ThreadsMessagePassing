#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>

#include "../common/message.hpp"

namespace TMQ
{
	class Queue
	{
		std::mutex _mutex;
		std::condition_variable _condition;
		std::queue<std::shared_ptr<MessageBase>> _queue;
	public:
		template<typename T>
		void push(const T &message)
		{
			std::lock_guard<std::mutex> lock(_mutex);
			_queue.push(std::make_shared<Message<T>>(message));
			_condition.notify_one(); 
		}
		std::shared_ptr<MessageBase> waitAndPop()
		{
			std::unique_lock<std::mutex> lock(_mutex);
			_condition.wait(lock, [&]{return !_queue.empty(); });
			auto res = _queue.front();
			_queue.pop();
			return res;
		}
	};
}