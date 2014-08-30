#pragma once

#include <cstdlib>
#include <new>
#include <utility>
#include <cinttypes>

namespace TMQ
{
	namespace PtrQueueType
	{
		static std::uint16_t __counter = 0;
		struct Base
		{
			Base(std::uint16_t _uid = 0)
			: uid(_uid)
			{
			}
			virtual ~Base(){}
			const std::uint16_t uid;
		};
		template <typename T>
		struct BaseUid : public Base
		{
			static const std::uint16_t getId() { static uint16_t id = __counter++; return id; }
			BaseUid()
				: Base(BaseUid::getId())
			{}
			virtual ~BaseUid(){}
		};
	}

	template <typename Base>
	class PtrQueue
	{
	public:
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

		PtrQueue(const PtrQueue &o) = delete;
		PtrQueue(PtrQueue &&o) = delete;
		PtrQueue&& operator=(PtrQueue &&o) = delete;

		~PtrQueue()
		{
			if (_data != nullptr)
				free(_data);
		}

		template <typename T, typename ...Args>
		T* push(Args ...args)
		{
			if (_data == nullptr
				|| _size - _to < sizeof(T)+sizeof(std::size_t))
			{
				allocate<T>();
			}
			std::size_t s = sizeof(T);
			std::size_t sizeOfInt = sizeof(std::size_t);

			char *tmp = _data;
			tmp += _to;
			memcpy(tmp, &s, sizeOfInt);
			tmp += sizeOfInt;
			T* res = new(tmp)T(args...);
			_to += sizeOfInt + s;
			return res;
		}

		Base *pop()
		{
			if (empty())
				return nullptr;

			char *tmp = _data;
			std::size_t soi = sizeof(std::size_t);

			tmp += _cursor;
			std::size_t s = *reinterpret_cast<std::size_t*>(tmp);
			_cursor += s + soi;
			tmp += soi;
			return ((Base*)(tmp));
		}

		Base *front()
		{
			if (empty())
				return nullptr;

			char *tmp = _data;
			std::size_t soi = sizeof(std::size_t);

			tmp += _cursor;
			std::size_t s = *reinterpret_cast<std::size_t*>(tmp);
			tmp += soi;
			return ((Base*)(tmp));
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
		void allocate()
		{
			std::size_t sizeOfType = sizeof(T);
			
			while (_size - _to <= sizeOfType + sizeof(std::size_t)
				|| _size <= _to)
			{
				_size += _chunkSize;
				_data = (char*)(realloc(_data, _size));
			}
		}
	private:
		std::size_t _chunkSize;
		char *_data;
		std::size_t _cursor;
		std::size_t _size;
		std::size_t _to;
	};
}