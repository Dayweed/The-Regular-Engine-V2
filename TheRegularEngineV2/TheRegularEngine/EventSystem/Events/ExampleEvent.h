#ifndef EXAMPLE_EVENT_H
#define EXAMPLE_EVENT_H
//Step 1: Have these 2 header files in your event file
#include "pch.h"
#include "EventBase.h"

namespace TRE
{
	//Step2: Inherit from event
	struct ExampleEvent : Event
	{
		//variables
		char _x;
		char _y;
		//Step3: Constructor and do wtv the function needs to be done in here
		ExampleEvent(char x, char y) : _x(x), _y(y) {}
		//Step4: After that you need to publish the event as shown in InputHandler.cpp
		//Step5: The system that needs to listen to this event needs to subscribe to it in the init of the system
		//Like this EventHandler::Subscribe<ExampleEvent>(this, &T::ExampleEvent);
		
	};
}
#endif