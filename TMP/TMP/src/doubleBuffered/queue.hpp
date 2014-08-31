#pragma once

#include <cstdlib>
#include <new>
#include <utility>
#include <cinttypes>

#include "../common/message.hpp"

namespace TMQ
{
	namespace Double
	{

		class Queue;

			////////////////////////////
			////// PTR QUEUE
		class PtrQueue
		{
		public:
			PtrQueue(const PtrQueue &o) = delete;
			PtrQueue(PtrQueue &&o) = delete;
			PtrQueue&& operator=(PtrQueue &&o) = delete;

			PtrQueue(std::size_t chunkSize = 1024)
				: _chunkSize(chunkSize)
				, _data(nullptr)
				, _cursor(0)
				, _size(0)
				, _to(0)
			{
			}

			PtrQueue& operator=(const PtrQueue &o)
			{
				clear();
				if (_size < o._size)
					_data = (char*)(realloc(_data, o._size));
				_chunkSize = o._chunkSize;
				_data = (char*)memcpy((void*)_data, (void*)o._data, o._size);
				_cursor = o._cursor;
				_size = o._size;
				_to = o._to;
				return *this;
			}

			~PtrQueue()
			{
				if (_data != nullptr)
					free(_data);
			}

			MessageBase *pop()
			{
				if (empty())
					return nullptr;

				char *tmp = _data;
				std::size_t soi = sizeof(std::size_t);

				tmp += _cursor;
				std::size_t s = *reinterpret_cast<std::size_t*>(tmp);
				_cursor += s + soi;
				tmp += soi;
				return ((MessageBase*)(tmp));
			}

			MessageBase *front()
			{
				if (empty())
					return nullptr;

				char *tmp = _data;
				std::size_t soi = sizeof(std::size_t);

				tmp += _cursor;
				std::size_t s = *reinterpret_cast<std::size_t*>(tmp);
				tmp += soi;
				return ((MessageBase*)(tmp));
			}

			void clear()
			{
				_cursor = _to = 0;
			}

			void release()
			{
				clear();
				if (_data != nullptr)
					free(_data);
				_data = nullptr;
			}

			bool empty()
			{
				if (_cursor >= _to)
				{
					_cursor = 0;
					_to = 0;
					return true;
				}
				return false;
			}

		private:
			template <typename T>
			T* push(const T& e)
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
				Message<T>* res = new(tmp)Message<T>(T(args...));
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
			////// !PTR QUEUE
			////////////////////////////




		class Queue
		{
			PtrQueue _queue;
			PtrQueue _copy;
			std::mutex _mutex;
			std::condition_variable _readCondition;
			std::condition_variable _writeCondition;

		public:
			Queue()
			{
			}

			void launch()
			{
				_writeCondition.notify_one();
			}

			Queue(const Queue&) = delete;
			Queue &operator=(const Queue&) = delete;
			Queue(Queue &&) = delete;
			Queue operator=(Queue &&) = delete;

			void getReadableQueue(PtrQueue& q)
			{
				std::unique_lock<std::mutex> lock(_mutex);
				_readCondition.wait(lock, [this](){ return !_copy.empty(); });
				q = _copy;
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

			//////
			////// Internal queue access

			template <typename T>
			T* push(const T& e)
			{
				return _queue.push(e);
			}

			template <typename T, typename ...Args>
			T* emplace(Args ...args)
			{
				return _queue.emplace<T>(args...);
			}
		};
	}
}