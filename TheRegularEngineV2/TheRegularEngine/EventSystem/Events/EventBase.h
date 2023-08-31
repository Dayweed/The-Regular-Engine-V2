#ifndef EVENT_BASE_H
#define EVENT_BASE_H

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

#endif