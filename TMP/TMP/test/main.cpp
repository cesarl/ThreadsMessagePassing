#include "../src/queue.hpp"
#include "../src/templateDispatcher.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <fstream>

struct StringMessage
{
	std::string data;
	StringMessage(const std::string &_data)
		: data(_data)
	{}
};

struct IntMessage
{
	int data;
	IntMessage(int _data)
		: data(_data)
	{}
};

struct FutureString : public TMQ::FutureData < std::string >, StringMessage
{
	FutureString(const std::string &_data)
		: StringMessage(_data)
	{}
};

struct FutureInt : public TMQ::FutureData < int >, IntMessage
{};

class Test
{
	TMQ::Queue _queue;
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
				.handle<StringMessage>([&](const StringMessage& msg)
			{
				a_file << "String message : " << msg.data << std::endl;
				counter++;
			})
				.handle<IntMessage>([&](const IntMessage& msg)
			{
				a_file << "Int message : " << msg.data << std::endl;
				counter++;
			})
				.handle<FutureInt>([&](FutureInt& msg)
			{
				msg.result.set_value(42);
				a_file << "Treating future int" << std::endl;
			})
				.handle<FutureString>([&](FutureString& msg)
			{
				a_file << "Treating future string : " << msg.data << std::endl;
				msg.result.set_value("Threated ! : " + msg.data);
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
		for (auto i = 0; i < 10; ++i)
		{
			for (auto j = 0; j < 10; ++j)
			{
				_queue.emplace<StringMessage>("String message A [" + std::to_string(i) + "][" + std::to_string(j) + "]\n");
				_queue.emplace<StringMessage>("String message B [" + std::to_string(i) + "][" + std::to_string(j) + "]\n");
				_queue.emplace<StringMessage>("String message C [" + std::to_string(i) + "][" + std::to_string(j) + "]\n");
				auto future1 = _queue.emplaceFuture<FutureString, std::string>(" Future String message [" + std::to_string(i) + "][" + std::to_string(j) + "]\n");
				auto future2 = _queue.priorityFutureEmplace<FutureString, std::string>(" Priority future String message [" + std::to_string(i) + "][" + std::to_string(j) + "]\n");
			}
			_queue.releaseReadability();
		}
		done();
	}

	Test()
	{
	}

	void done()
	{
		_queue.emplace<TMQ::CloseQueue>();
		_queue.releaseReadability();
	}

	~Test()
	{
		done();
	}
};

int main(void)
{
	{
		auto start = std::chrono::high_resolution_clock::now();
		Test test;
		std::thread worker(&Test::runWorker, &test);
		std::thread main(&Test::runEmitter, &test);
		main.join();
		worker.join();
		auto end = std::chrono::high_resolution_clock::now();
		auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		std::ofstream a_file("LOG.txt", std::ofstream::app);
		a_file << "Duration Double : " << std::to_string(dur.count()) << std::endl;
	}
	return 0;
}