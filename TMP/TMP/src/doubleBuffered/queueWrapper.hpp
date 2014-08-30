#pragma once

#include "queue.hpp"

namespace TMQ
{
	template <typename QueueType>
	class QueueWrapper
	{
		TMQ::PtrQueue<QueueType> _queue;
		TMQ::PtrQueue<QueueType> _copy;
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

		void releaseReadability()
		{
			std::unique_lock<std::mutex> lock(_mutex);
			_writeCondition.wait(lock);
			_copy = _queue;
			_readCondition.notify_one();
			lock.unlock();
		}

		PtrQueue<QueueType> &getWritableQueue()
		{
			return _queue;
		}

		void getReadableQueue(PtrQueue<QueueType> &queue)
		{
			std::unique_lock<std::mutex> lock(_mutex);
			_readCondition.wait(lock);
			queue = _copy;
			_writeCondition.notify_one();
			lock.unlock();
		}
	};
}