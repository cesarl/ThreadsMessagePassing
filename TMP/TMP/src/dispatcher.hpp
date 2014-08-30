#pragma once

#include "emitter.hpp"

namespace TMQ
{
	class CloseQueue
	{
	};

	class Dispatcher
	{
		TMQ::Queue *_queue;
		bool _chained;

		Dispatcher(const Dispatcher &) = delete;
		Dispatcher &operator=(const Dispatcher &) = delete;

		template < typename _Dispatcher
			, typename _Message
			, typename _Func>
			friend class TemplateDispatcher;


		void waitAndDispatch()
		{
			while (true)
			{
				auto message = _queue->waitAndPop();
				dispatch(message);
			}
		}

		bool dispatch(const std::shared_ptr<MessageBase> &msg)
		{
			if (dynamic_cast<Message<CloseQueue>*>(msg.get()))
			{
				throw CloseQueue();
			}
			return false;
		}

	public:
		Dispatcher(Dispatcher &&o) :
			_queue(o._queue)
			, _chained(o._chained)
		{
			o._chained = false;
		}

		explicit Dispatcher(TMQ::Queue *queue)
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