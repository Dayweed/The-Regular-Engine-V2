#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H
#include "pch.h"
#include "../Events/EventBase.h"

namespace TRE
{
	class HandlerFunctionBase
	{
	public:
		virtual ~HandlerFunctionBase() = default;

		void exec(Event& event)
		{
			call(event);
		}

	private:
		virtual void call(Event& event) = 0;
	};

	template <typename T, typename EventType>
	class MemberFunctionHandler : public HandlerFunctionBase
	{
	private:
		using MemberFunction = void (T::*)(EventType&);
		//Pointer to the class instance
		T* _instance;
		//Pointer to the member function
		MemberFunction _memberFunction;
	public:
		//Default ctor
		MemberFunctionHandler() : _instance{nullptr}, _memberFunction{nullptr}
		{}
		//User defined ctor
		MemberFunctionHandler(T* instance, MemberFunction memberFunction) :
			_instance{instance}, _memberFunction{memberFunction}
		{}
		//Calls the response function
		void call(Event& event) override
		{
			(_instance->*_memberFunction)(static_cast<EventType&>(event));
		}
	};

	class EventHandler
	{
	private:
		using HandlerList = std::vector<std::unique_ptr<HandlerFunctionBase>>;
		//Map of (type of event) - (Subscribers)
		std::unordered_map<std::type_index, HandlerList> _subscribers;
	public:
		//Singleton Class
		static EventHandler& getEventHandlerInstance()
		{
			static EventHandler instance{};
			return instance;
		}

		template<typename EventType>
		void publish(EventType&& event)
		{
			//Get the handler based on type index / type id of the event
			const auto& delegates = _subscribers[typeid(EventType)];
			for (auto& handler : delegates)
				if (handler != nullptr) handler->exec(event);
		}

		template<typename T, typename EventType>
		void subscribe(T* instance, void(T::* memberFunction)(EventType&))
		{
			_subscribers[typeid(EventType)].emplace_back
			(
				std::make_unique<MemberFunctionHandler<T, EventType>>(instance, memberFunction)
			);
		}
	};
}

#endif