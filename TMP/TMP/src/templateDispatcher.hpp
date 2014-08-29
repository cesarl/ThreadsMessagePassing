#pragma once

#include "dispatcher.hpp"

namespace TMQ
{
	template <typename _PreviousDispatcher,
		typename _Message,
		typename _Function>
	class TemplateDispatcher
	{
		TMQ::Queue* _queue;
		_PreviousDispatcher* _previous;
		_Function _function;
		bool _chained;

		TemplateDispatcher(const TemplateDispatcher &) = delete;
		TemplateDispatcher& operator=(const TemplateDispatcher &) = delete;

		template<typename _Dispatcher, typename _Message_2, typename _Function_2>
		friend class TemplateDispatcher;

		void waitAndDispatch()
		{
			while (true)
			{
				auto message = _queue->waitAndPop();
				if (dispatch(message))
					break;
			}
		}

		bool dispatch(const std::shared_ptr<Message>& msg)
		{
			if (Message<_Message>* wrapper = dynamic_cast<Message<_Message>*>(msg.get()))
			{
				_function(wrapper->_data);
				return true;
			}
			else
				return _previous->dispatch(msg);
		}

	public:

		TemplateDispatcher(TemplateDispatcher&& o)
			: _queue(o._queue)
			, _function(o._function)
			, _chained(o._chained)
		{
			o._chained = true;
		}

		TemplateDispatcher(TMQ::Queue* queue, _PreviousDispatcher* previous, _Function&& f)
			: _queue(queue)
			, _previous(previous)
			, _function(std::forward<_Function>(f))
			, _chained(false)
		{
			previous->_chained = true;
		}

		template<typename _Message_2, typename _Function_2>
		TemplateDispatcher<TemplateDispatcher, _Message_2, _Function_2>
		handle(_Function_2&& function)
		{
			return TemplateDispatcher<TemplateDispatcher, _Message_2, _Function_2>(_queue, this, std::forward<_Function_2>(function));
		}

		~TemplateDispatcher() noexcept(false)
		{
			if (!_chained)
				waitAndDispatch();
		}
	};
}