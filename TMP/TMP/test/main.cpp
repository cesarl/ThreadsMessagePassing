#include "../src/singleBuffered/templateDispatcher.hpp"
#include "../src/singleBuffered/receiver.hpp"

#include "../src/doubleBuffered/queue.hpp"
#include "../src/doubleBuffered/queueWrapper.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <fstream>

enum OperationType
{
};

struct DuoMessage
{
	int a;
	int b;

	DuoMessage(int _a, int _b)
		: a(_a), b(_b)
	{}
};

struct TrioMessage
{
	int a;
	int b;
	int c;

	TrioMessage(int _a, int _b, int _c)
		: a(_a), b(_b), c(_c)
	{}
};

struct QuatroMessage
{
	int a;
	int b;
	int c;
	int d;

	QuatroMessage(int _a, int _b, int _c, int _d)
		: a(_a), b(_b), c(_c), d(_d)
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
					.handle<DuoMessage>([&](const DuoMessage& msg)
				{
					auto res = std::sqrt(msg.a) * std::acos(msg.b);
				})
					.handle<TrioMessage>([&](const TrioMessage& msg)
				{
					auto res = std::sqrt(msg.a) * std::acos(msg.b) * std::asin(msg.c);
				})
					.handle<QuatroMessage>([&](const QuatroMessage& msg)
				{
					auto res = std::sqrt(msg.a) * std::acos(msg.b) * std::asin(msg.c) * std::sin(msg.d);
				});
			}
		}
		catch (const TMQ::CloseQueue &)
		{
		}
	}

	void runEmitter()
	{
		std::srand(42);
		for (auto i = 0; i < 1000000; ++i)
		{
			auto res = std::sqrt(rand()) * std::acos(rand()) * std::asin(rand()) * std::sin(rand());
			if (i % 2)
			{
				_emitter.send(DuoMessage(res, rand()));
			}
			else if (i % 3)
			{

				_emitter.send(TrioMessage(res, rand(), rand()));
			}
			else
			{

				_emitter.send(QuatroMessage(res, rand(), rand(), rand()));
			}
		}
		done();
	}
	
	Test()
	{
		_emitter = _worker.operator TMQ::Emitter();
	}

	void done()
	{
		_emitter.send(TMQ::CloseQueue());
	}

	~Test()
	{
		done();
	}

};

struct MessageDb1 : TMQ::PtrQueueType::Base
{
	int i = 42;
};

void DbWorker(TMQ::QueueWrapper<TMQ::PtrQueueType::Base> &queue)
{
	auto ii = 0;
	while (ii++ < 100)
	{
		TMQ::PtrQueue<TMQ::PtrQueueType::Base> q;
		queue.getReadableQueue(q);
		while (!q.empty())
		{
			auto v = q.pop();
			std::cout << ((MessageDb1*)(v))->i << std::endl;
		}
	}
}

void DbMain(TMQ::QueueWrapper<TMQ::PtrQueueType::Base> &queue)
{
	auto ii = 0;
	while (ii++ < 100)
	{
		auto &q = queue.getWritableQueue();
		for (auto i = 0; i < 10; ++i)
			q.push<MessageDb1>();
		queue.releaseReadability();
	}
}

int main(void)
{
	//auto start = std::chrono::high_resolution_clock::now();
	//Test test;
	//std::thread main(&Test::runEmitter, &test);
	//std::thread worker(&Test::runWorker, &test);
	//main.join();
	//worker.join();
	//auto end = std::chrono::high_resolution_clock::now();
	//auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
	//std::ofstream a_file("LOG.txt", std::ofstream::app);
	//a_file << "Duration : " << std::to_string(dur.count()) << std::endl;


	//------------------------------

	TMQ::QueueWrapper<TMQ::PtrQueueType::Base> queue;

	std::thread main(DbMain, std::ref(queue));
	std::thread worker(DbWorker, std::ref(queue));
	queue.launch();
	main.join();
	worker.join();

	return 0;
}