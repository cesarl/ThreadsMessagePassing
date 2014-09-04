#pragma once

#include <future>

namespace TMQ
{
	struct MessageBase
	{
		virtual ~MessageBase(){}
		MessageBase(std::size_t _uid) : uid(_uid){}
		std::size_t uid;
	};

	namespace __Private
	{
		static std::size_t _messageID = 0;
	}

	template <typename T>
	struct Message : public MessageBase
	{
		T _data;

		static std::size_t getId()
		{
			static std::size_t id = __Private::_messageID++;
			return id;
		}

		explicit Message(const T &data)
			: MessageBase(getId()), _data(data)
		{}

		explicit Message(T &&data)
			: MessageBase(getId()), _data(std::move(data))
		{}

		template <typename ...Args>
		explicit Message(Args ...args)
			: MessageBase(getId()), _data(args...)
		{
		}
	};

	class CloseQueue
	{
	};
}