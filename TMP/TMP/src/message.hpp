#pragma once

namespace TMQ
{
	struct MessageBase
	{
		virtual ~MessageBase(){}
	};

	template <typename T>
	struct Message : public MessageBase
	{
		T _data;
		explicit Message(const T &data)
			: _data(data)
		{}
	};
}