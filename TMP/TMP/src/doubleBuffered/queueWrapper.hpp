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
		bool _readable;
		bool _writable;


	public:
		QueueWrapper()
			: _readable(false)
			, _writable(false)
		{
		}

		void launch()
		{
			_writable = true;
			_writeCondition.notify_one();
		}

		QueueWrapper(const QueueWrapper&) = delete;
		QueueWrapper &operator=(const QueueWrapper&) = delete;

		// todo move operators

		void releaseReadability()
		{
			std::unique_lock<std::mutex> lock(_mutex);
			_writeCondition.wait(lock, [&](){return _writable; });
			_readable = false;
			_copy = _queue;
			_queue.clear();
			_readable = true;
			lock.unlock();
			_readCondition.notify_one();
		}

		PtrQueue<QueueType> &getWritableQueue()
		{
			return _queue;
		}

		void getReadableQueue(PtrQueue<QueueType> &queue)
		{
			std::unique_lock<std::mutex> lock(_mutex);
			_writable = false;
			_readCondition.wait(lock, [&](){return _readable; });
			queue = _copy;
			_copy.clear();
			_writable = true;
			lock.unlock();
			_writeCondition.notify_one();
		}
	};
}