#pragma once

#include "queue.hpp"

namespace TMQ
{
	class Emitter
	{
		Queue *_queue;
	public:
		Emitter() :
			_queue(nullptr)
		{}

		explicit Emitter(Queue *queue)
			: _queue(queue)
		{}

		template<typename T>
		void send(const T &message)
		{
			_queue->push(message);
		}
	};
}