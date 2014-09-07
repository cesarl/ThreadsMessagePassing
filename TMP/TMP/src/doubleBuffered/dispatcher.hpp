#pragma once

#include "queue.hpp"

namespace TMQ
{
	namespace Double
	{
		class Dispatcher
		{
			TMQ::Double::Queue *_queue;
			bool _chained;

			Dispatcher(const Dispatcher &) = delete;
			Dispatcher &operator=(const Dispatcher &) = delete;

			template < typename _Dispatcher
				, typename _Message
				, typename _Func>
				friend class TemplateDispatcher;

			void waitAndDispatch()
			{
				TMQ::Double::PtrQueue q;
				_queue->getReadableQueue(q);
				while (!q.empty())
				{
					auto message = q.front();
					dispatch(message);
					q.pop();
				}
			}

			bool dispatch(MessageBase* &msg)
			{
				return false;
			}

		public:
			Dispatcher(Dispatcher &&o) :
				_queue(o._queue)
				, _chained(o._chained)
			{
				o._chained = false;
			}

			explicit Dispatcher(TMQ::Double::Queue *queue)
				: _queue(queue)
				, _chained(false)
			{}

			template < typename _Message
				, typename _Func>
				TemplateDispatcher<Dispatcher, _Message, _Func>
				handle(_Func &&f)
			{
				return TemplateDispatcher<Dispatcher, _Message, _Func>(_queue, this, std::forward<_Func>(f));
			}

			~Dispatcher()// noexcept(false)
			{
				if (!_chained)
				{
					waitAndDispatch();
				}
			}
		};
	}
}