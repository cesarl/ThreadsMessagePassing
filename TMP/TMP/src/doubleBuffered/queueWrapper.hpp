#pragma once

#include "queue.hpp"

namespace TMQ
{
	class QueueWrapper
	{
		TMQ::PtrQueue _queue;
		TMQ::PtrQueue _copy;
		std::mutex _mutex;
		std::condition_variable _readCondition;
		std::condition_variable _writeCondition;

	public:
		QueueWrapper()
		{
		}

		void launch()
		{
			_writeCondition.notify_one();
		}

		QueueWrapper(const QueueWrapper&) = delete;
		QueueWrapper &operator=(const QueueWrapper&) = delete;

		// todo move operators

		PtrQueue &getWritableQueue()
		{
			return _queue;
		}

		void getReadableQueue(PtrQueue &queue)
		{
			std::unique_lock<std::mutex> lock(_mutex);
			_readCondition.wait(lock, [this](){ return !_copy.empty(); });
			queue = _copy;
			_copy.clear();
			lock.unlock();
			_writeCondition.notify_one();
		}

		void releaseReadability()
		{
			std::unique_lock<std::mutex> lock(_mutex);
			_writeCondition.wait(lock, [this](){ return _copy.empty(); });
			_copy = _queue;
			_queue.clear();
			lock.unlock();
			_readCondition.notify_one();
		}

	};
}