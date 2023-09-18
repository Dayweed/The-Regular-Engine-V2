#pragma once

namespace TRE
{
	//Base Event that all future events inherit from
	class Event
	{
	public:
		virtual ~Event() = 0
		{}
	};
}
