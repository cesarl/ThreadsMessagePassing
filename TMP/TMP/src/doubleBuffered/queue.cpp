#include "queue.hpp"
#include "dispatcher.hpp"

namespace TMQ
{
	namespace Double
	{
			Dispatcher Queue::getDispatcher()
			{
				return Dispatcher(this);
			}
	}
}

