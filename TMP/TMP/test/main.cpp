#include "../src/singleBuffered/templateDispatcher.hpp"
#include "../src/singleBuffered/receiver.hpp"

#include "../src/doubleBuffered/queue.hpp"
#include "../src/doubleBuffered/templateDispatcher.hpp"

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
	int d : 4;
	int e : 3;
	int f : 2;
	bool g;

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

struct FutureMessage : public TMQ::FutureData<int>
{};

//class Test
//{
//	TMQ::Single::Receiver _worker;
//	TMQ::Single::Emitter _emitter;
//
//public:
//	void runWorker()
//	{
//		bool run = true;
//		auto counter = 0;
//		std::ofstream a_file("TEST.txt");
//		while (run)
//		{
//			auto res = 0;
//			_worker.getDispatcher()
//				.handle<DuoMessage>([&](const DuoMessage& msg)
//			{
//				res = msg.a * msg.b;
//				a_file << res << ", ";
//				counter++;
//			})
//				.handle<TrioMessage>([&](const TrioMessage& msg)
//			{
//				res = msg.a * msg.b * msg.c;
//				a_file << res << ", ";
//				counter++;
//			})
//				.handle<QuatroMessage>([&](const QuatroMessage& msg)
//			{
//				res = msg.a * msg.b * msg.c * msg.d;
//				a_file << res << ", ";
//				counter++;
//			})
//				.handle<TMQ::CloseQueue>([&](const TMQ::CloseQueue& msg)
//			{
//				run = false;
//			});
//		}
//		std::cout << counter << std::endl;
//	}
//
//	void runEmitter()
//	{
//		std::srand(42);
//		for (auto i = 0; i < 1000; ++i)
//		{
//			for (auto j = 0; j < 1000; ++j)
//			{
//				auto res = rand();
//				if (i % 2)
//				{
//					_emitter.send(DuoMessage(res, rand()));
//				}
//				else if (i % 3)
//				{
//
//					_emitter.send(TrioMessage(res, rand(), rand()));
//				}
//				else
//				{
//					_emitter.send(QuatroMessage(res, rand(), rand(), rand()));
//				}
//			}
//		}
//		done();
//	}
//
//	Test()
//	{
//		_emitter = _worker.operator TMQ::Single::Emitter();
//	}
//
//	void done()
//	{
//		_emitter.send(TMQ::CloseQueue());
//	}
//
//	~Test()
//	{
//		done();
//	}
//};

class Test2
{
	TMQ::Double::Queue _queue;
public:
	void runWorker()
	{
		bool run = true;
		auto counter = 0;
		std::ofstream a_file("TEST.txt");
		while (run)
		{
			int res = 0;
			_queue.getDispatcher()
				.handle<DuoMessage>([&](const DuoMessage& msg)
			{
				res = msg.a * msg.b;
				a_file << res << ", ";
				counter++;
			})
				.handle<TrioMessage>([&](const TrioMessage& msg)
			{
				res = msg.a * msg.b * msg.c;
				a_file << res << ", ";
				counter++;
			})
				.handle<QuatroMessage>([&](const QuatroMessage& msg)
			{
				res = msg.a * msg.b * msg.c * msg.d;
				a_file << res << ", ";
				counter++;
			})
				.handle<FutureMessage>([&](FutureMessage& msg)
			{
				msg.result.set_value(42);
				a_file << std::endl << "Treating future" << std::endl;
			})
				.handle<TMQ::CloseQueue>([&](const TMQ::CloseQueue& msg)
			{
				run = false;
			});
		}
		std::cout << counter << std::endl;
	}

	void runEmitter()
	{
		_queue.launch();
		std::srand(42);
		for (auto i = 0; i < 100; ++i)
		{
			for (auto j = 0; j < 10; ++j)
			{
				auto res = rand();
				if (i % 2)
				{
					_queue.emplace<DuoMessage>(res, rand());
				}
				else if (i % 3)
				{
					_queue.emplace<TrioMessage>(res, rand(), rand());
				}
				else
				{
					_queue.emplace<QuatroMessage>(res, rand(), rand(), rand());
				}

				if (j % 3 == 0)
				{
					auto futureInt = _queue.priorityEmplace<FutureMessage, int>();
					//auto v = futureInt.get();
					//std::cout << "Priority with future when i = " << i << " and j = " << j << " is : " << v << std::endl;
				}
			}
			_queue.releaseReadability();
		}
		done();
	}

	Test2()
	{
	}

	void done()
	{
		_queue.emplace<TMQ::CloseQueue>();
		_queue.releaseReadability();
	}

	~Test2()
	{
		done();
	}
};

//struct MessageDb1
//{
//	int i = 42;
//	MessageDb1(int _i) : i(_i){}
//};
//
//void DbWorker(TMQ::Double::Queue &queue)
//{
//	TMQ::Double::PtrQueue q;
//	while (true)
//	{
//		queue.getReadableQueue(q);
//		while (!q.empty())
//		{
//			auto v = q.pop();
//			std::cout << ((TMQ::Message<MessageDb1>*)(v))->_data.i << std::endl;
//		}
//	}
//}
//
//void DbMain(TMQ::Double::Queue &queue)
//{
//	auto ii = 0;
//	while (ii < 10)
//	{
//		for (auto i = 0; i < 10; ++i)
//		{
//			queue.emplace<MessageDb1>(ii * 100000 + i);
//		}
//		queue.releaseReadability();
//		++ii;
//	}
//}

int main(void)
{
	{
		//auto start = std::chrono::high_resolution_clock::now();
		//Test test;
		//std::thread main(&Test::runEmitter, &test);
		//std::thread worker(&Test::runWorker, &test);
		//main.join();
		//worker.join();
		//auto end = std::chrono::high_resolution_clock::now();
		//auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		//std::ofstream a_file("LOG.txt", std::ofstream::app);
		//a_file << "Duration Single : " << std::to_string(dur.count()) << std::endl;
	}

	//------------------------------

{
	auto start = std::chrono::high_resolution_clock::now();
	Test2 test;
	std::thread worker(&Test2::runWorker, &test);
	std::thread main(&Test2::runEmitter, &test);
	main.join();
	worker.join();
	auto end = std::chrono::high_resolution_clock::now();
	auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::ofstream a_file("LOG.txt", std::ofstream::app);
	a_file << "Duration Double : " << std::to_string(dur.count()) << std::endl;
}
	return 0;
}