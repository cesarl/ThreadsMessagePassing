#pragma once

#include "message.hpp"

#include <cstdlib>
#include <new>
#include <utility>
#include <cinttypes>
#include <condition_variable>
#include <mutex>

namespace TMQ
{
	class Queue;
	class Dispatcher;
	class Messsage;

	class PtrQueue
	{
	public:
		PtrQueue(const PtrQueue &o) = delete;
		PtrQueue& operator=(const PtrQueue &o) = delete;
		PtrQueue(std::size_t chunkSize = 1024);
		PtrQueue& operator=(PtrQueue &&o);
		PtrQueue(PtrQueue &&o);
		~PtrQueue();
		void pop();
		MessageBase *front();
		void clear();
		void release();
		bool empty();
	private:
		template <typename T>
		T* push(T&& e)
		{
			std::size_t s = sizeof(Message<T>);
			std::size_t sizeOfInt = sizeof(std::size_t);

			if (_data == nullptr
				|| _size - _to < s + sizeOfInt)
			{
				allocate<T>();
			}

			char *tmp = _data;
			tmp += _to;
			memcpy(tmp, &s, sizeOfInt);
			tmp += sizeOfInt;
			Message<T>* res = new(tmp)Message<T>(e);
			_to += sizeOfInt + s;
			return &res->_data;
		}

		template <typename T, typename ...Args>
		T* emplace(Args ...args)
		{
			std::size_t s = sizeof(Message<T>);
			std::size_t sizeOfInt = sizeof(std::size_t);

			if (_data == nullptr
				|| _size - _to < s + sizeOfInt)
			{
				allocate<T>();
			}

			char *tmp = _data;
			tmp += _to;
			memcpy(tmp, &s, sizeOfInt);
			tmp += sizeOfInt;
			Message<T>* res = new(tmp)Message<T>(args...);
			_to += sizeOfInt + s;
			return &res->_data;
		}

		template <typename T>
		void allocate()
		{
			std::size_t sizeOfType = sizeof(Message<T>);

			while (_size - _to <= sizeOfType + sizeof(std::size_t)
				|| _size <= _to)
			{
				_size += _chunkSize;
				_data = (char*)(realloc(_data, _size));
			}
		}

		std::size_t _chunkSize;
		char *_data;
		std::size_t _cursor;
		std::size_t _size;
		std::size_t _to;

		friend class Queue;
	};

	class Queue
	{
		PtrQueue _queue;
		PtrQueue _copy;
		PtrQueue _priority;
		PtrQueue _priorityCopy;
		std::mutex _mutex;
		std::condition_variable _readCondition;
		std::condition_variable _writeCondition;
	public:
		Queue();
		void launch();

		Queue(const Queue&) = delete;
		Queue &operator=(const Queue&) = delete;
		Queue(Queue &&) = delete;
		Queue operator=(Queue &&) = delete;

		void getReadableQueue(PtrQueue& q);
		Dispatcher getDispatcher();
		void releaseReadability();

		//////
		////// Internal standard queue access

		//Do not lock mutex
		//Use it only if used in the same thread, or use safePush
		template <typename T>
		T* push(const T& e)
		{
			return _queue.push(e);
		}

		//Do not lock mutex
		//Use it only if used in the same thread, or use safeEmplace
		template <typename T, typename ...Args>
		T* emplace(Args ...args)
		{
			return _queue.emplace<T>(args...);
		}

		//Lock mutex
		//Can be called simutanously in different threads
		template <typename T>
		void safePush(const T& e)
		{
			std::lock_guard<std::mutex> lock(_mutex);
			_queue.push(e);
		}

		//Lock mutex
		//Can be called simutanously in different thre
		template <typename T, typename ...Args>
		void safeEmplace(Args ...args)
		{
			std::lock_guard<std::mutex> lock(_mutex);
			_queue.emplace<T>(args...);
		}

		//Push in queue and return future
		//Message data have to heritate from FutureData
		template <typename T, typename F>
		std::future<F> pushFuture(const T &e)
		{
			return _queue.push(e)->getFuture();
		}

		//Emplace in queue and return future
		//Message data have to heritate from FutureData
		template <typename T, typename F, typename ...Args>
		std::future<F> emplaceFuture(Args ...args)
		{
			return _queue.emplace<T>(args...)->getFuture();
		}

		//////
		////// Internal priority queue access

		//Lock mutex
		//Can be called simutanously in different threads
		template <typename T>
		void priorityPush(const T& e)
		{
			{
				std::lock_guard<std::mutex> lock(_mutex);
				_queue.push(e);
			}
			releaseReadability();
		}

		//Lock mutex
		//Can be called simutanously in different thre
		template <typename T, typename ...Args>
		void priorityEmplace(Args ...args)
		{
			{
				std::lock_guard<std::mutex> lock(_mutex);
				_queue.emplace<T>(args...);
			}
			releaseReadability();
		}

		//Push in queue and return future
		//Message data have to heritate from FutureData
		template <typename T, typename F>
		std::future<F> priorityFuturePush(const T &e)
		{
			std::future<F> futur;
			{
				std::lock_guard<std::mutex> lock(_mutex);
				futur = _priority.push(e)->getFuture();
			}
			releaseReadability();
			return futur;
		}

		//Emplace in queue and return future
		//Message data have to heritate from FutureData
		template <typename T, typename F, typename ...Args>
		std::future<F> priorityFutureEmplace(Args ...args)
		{
			std::future<F> futur;
			{
				std::lock_guard<std::mutex> lock(_mutex);
				futur = (_priority.emplace<T>(args...))->getFuture();
			}
			releaseReadability();
			return futur;
		}
	};
}