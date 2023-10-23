#pragma once
#include "pch.h"
#include "EventSystem/Events/EventBase.h"

namespace TRE
{
	class HandlerFunctionBase
	{
	public:
		virtual ~HandlerFunctionBase() = default;

		void Exec(Event& event)
		{
			Call(event);
		}

	private:
		virtual void Call(Event& event) = 0;
	};

	template <typename T, typename EventType>
	class MemberFunctionHandler : public HandlerFunctionBase
	{
	private:
		using MemberFunction = void (T::*)(EventType&);
		//Pointer to the class instance
		T* m_Instance;
		//Pointer to the member function
		MemberFunction m_MemberFunction;
	public:
		//Default ctor
		MemberFunctionHandler() : m_Instance{nullptr}, m_MemberFunction{nullptr}
		{}
		//User defined ctor
		MemberFunctionHandler(T* instance, MemberFunction memberFunction) :
			m_Instance{instance}, m_MemberFunction{memberFunction}
		{}
		//Calls the response function
		void Call(Event& event) override
		{
			(m_Instance->*m_MemberFunction)(static_cast<EventType&>(event));
		}
	};

	class EventHandler
	{
	private:
		using HandlerList = std::vector<std::unique_ptr<HandlerFunctionBase>>;
		//Map of (type of event) - (Subscribers)
		std::unordered_map<std::type_index, HandlerList> m_Subscribers;
	public:
		//Singleton Class
		static EventHandler& getEventHandlerInstance()
		{
			static EventHandler instance{};
			return instance;
		}

		template<typename EventType>
		void Publish(EventType&& event)
		{
			//Get the handler based on type index / type id of the event
			const auto& delegates = m_Subscribers[typeid(EventType)];
			for (auto& handler : delegates)
				if (handler != nullptr) handler->Exec(event);
		}

		template<typename T, typename EventType>
		void subscribe(T* instance, void(T::* memberFunction)(EventType&))
		{
			m_Subscribers[typeid(EventType)].emplace_back
			(
				std::make_unique<MemberFunctionHandler<T, EventType>>(instance, memberFunction)
			);
		}
	};
}
