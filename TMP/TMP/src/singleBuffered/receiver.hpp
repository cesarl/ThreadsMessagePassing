#pragma once

#include "dispatcher.hpp"

namespace TMQ
{
	namespace Single
	{
		class Receiver
		{
			Queue _queue;
		public:
			operator Emitter()
			{
				return Emitter(&_queue);
			}

			Dispatcher getDispatcher()
			{
				return Dispatcher(&_queue);
			}
		};
	}
}