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

			Dispatcher wait()
			{
				return Dispatcher(&_queue);
			}
		};
	}
}