#include "../src/templateDispatcher.hpp"
#include "../src/receiver.hpp"

#include <iostream>
#include <string>

struct EmptyMessage
{};

struct StringMessage
{
	const std::string str;

	StringMessage(const std::string &s)
		: str(s)
	{}
};

struct NumberMessage
{
	int number;

	NumberMessage(int i)
	: number(i)
	{}
};

class Test
{
	TMQ::Receiver _worker;
	TMQ::Emitter _emitter;

public:
	void runWorker()
	{
		try
		{
			for (;;)
			{
				_worker.wait()
					.handle<StringMessage>([&](const StringMessage& msg)
				{
					std::cout << "String message : " << msg.str << std::endl;
				})
					.handle<NumberMessage>([&](const NumberMessage& msg)
				{
					std::cout << "Number message : " << msg.number << std::endl;
				})
					.handle<EmptyMessage>([&](const EmptyMessage& msg)
				{
					std::cout << "Empty message !" << std::endl;
				});
			}
		}
		catch (const TMQ::CloseQueue &)
		{
		}
	}

	void runEmitter()
	{
		_emitter.send(StringMessage("Ceci est un message string"));
		_emitter.send(EmptyMessage());
		_emitter.send(EmptyMessage());
		_emitter.send(EmptyMessage());
		for (auto i = 0; i < 10; ++i)
		{
			if (i % 2)
			{
				_emitter.send(NumberMessage(i));
			}
			else
				_emitter.send(StringMessage("Coucou " + std::to_string(i)));
		}
	}
};

int main(void)
{

	return 0;
}