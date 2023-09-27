/*!
	@file    Profiler.cpp
	@author  Isaiah Lim (Code Contribution 100%)
	@email   lim.i@digipen.edu
	@date    14/09/2022
	@brief   This file contains the structs that handles the timing taken for each systems

	Copyright (C) 2022 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/

/*                                                                   includes
----------------------------------------------------------------------------- */
#include "pch.h"
#include "Profiler.h"
#include <iostream>
#include <thread>
#include "EventSystem/EventHandler/EventHandler.h"
#include "EventSystem/Events/EditorEvent.h"

namespace TRE
{
	//==================================================
	/* !
	@function  Profiler::Instance
	@author    Isaiah Lim  (lim.i@digipen.edu)

	@params

	@brief	   Creates a singleton for the Profiler

	*//*__________________________________________________________________________*/
	Profiler& Profiler::Instance()
	{
		static Profiler instance;
		return instance;
	}

	// Timer creators
	//==================================================
	/* !
	@function  Profiler::StartTimer
	@author    Isaiah Lim  (lim.i@digipen.edu)

	@params

	@brief	   Creates a new timer if timer doesn't exist and start counting

	*//*__________________________________________________________________________*/
	void Profiler::StartTimer(std::string name_)
	{
		std::unordered_map<std::string, Timer*>::iterator timer{ timers.find(name_) };
		// Check if it exist
		if (timer != timers.end())
		{
			(*timer).second->StartTime();
		}
		// Create a new timer and start that
		else
		{
			Timer* t = new Timer();
			timers.insert({ name_, t });
			timers.find(name_)->second->StartTime();
		}
	}

	// Timer creators
	//==================================================
	/* !
	@function  Profiler::EndTimer
	@author    Isaiah Lim  (lim.i@digipen.edu)

	@params

	@brief	   End timer if it exists and return true else return false

	*//*__________________________________________________________________________*/
	bool Profiler::EndTimer(std::string name_)
	{
		std::unordered_map<std::string, Timer*>::iterator timer{ timers.find(name_) };
		// Check if it exist
		if (timer != timers.end())
		{
			(*timer).second->EndTime();
			return true;
		}
		return false;
	}

	//==================================================
	/* !
	@function  Profiler::RemoveTimer
	@author    Isaiah Lim  (lim.i@digipen.edu)

	@params

	@brief	   Finds the timer with that name and remove it, if it doesn't
			   exists it will return false

	*//*__________________________________________________________________________*/
	bool Profiler::RemoveTimer(std::string name_)
	{
		std::unordered_map<std::string, Timer*>::iterator timer{ timers.find(name_) };
		// Check if it exist
		if (timer != timers.end())
		{
			timers.erase(timer);
			return true;
		}
		return false;
	}

	// Set Get Debugging Mode
	void Profiler::SetDebugMode(bool mode_on_)
	{
		debug_mode = mode_on_;
	}

	bool Profiler::GetDebugMode() const
	{
		return debug_mode;
	}

	// Debugging purposes for Profiler
	//==================================================
	/* !
	@function  Profiler::PrintTimers
	@author    Isaiah Lim  (lim.i@digipen.edu)

	@params

	@brief	   Print out all the timers [Only works if debug_mode == true]

	*//*__________________________________________________________________________*/
	void Profiler::PrintTimers()
	{
		std::unordered_map<std::string, Timer*> tmp{};
		if (!debug_mode || std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_delay).count() < delay)
		{
			return;
		}
		//// Iteration 2 Print by percentage
		////=========================================================================
		totalTime = 0;
		for (std::unordered_map<std::string, Timer*>::iterator timer{ timers.begin() }; timer != timers.end(); ++timer)
		{
			totalTime += (*timer).second->GetTime();
		}
		std::string EditLabel = "class TRE::";
		for (std::unordered_map<std::string, Timer*>::iterator timer{ timers.begin() }; timer != timers.end(); ++timer)
		{
			std::string label = (*timer).first;
			if (label.find(EditLabel) != std::string::npos)
				label.erase(label.find(EditLabel), EditLabel.length());
			if((*timer).first != "Imgui")
				tmp.insert({ label, (*timer).second });
		}
		EventHandler::getEventHandlerInstance().Publish(SendTimeTakenEvent{tmp});
		start_delay = std::chrono::steady_clock::now();
	}

	std::unordered_map<std::string, Timer*>& Profiler::GetTimers()
	{
		return timers;
	}

	const long long Profiler::GetTotalTime() const
	{
		return totalTime;
	}

	// Ctor and Dtor
	//==================================================
	/* !
	@function  Profiler::Profiler
	@author    Isaiah Lim  (lim.i@digipen.edu)

	@params

	@brief	   Constructor

	*//*__________________________________________________________________________*/
	Profiler::Profiler()
	{

	}

	//==================================================
	/* !
	@function  Profiler::~Profiler
	@author    Isaiah Lim  (lim.i@digipen.edu)

	@params

	@brief	   Destructor

	*//*__________________________________________________________________________*/
	Profiler::~Profiler()
	{
		// Delete all timers
		for (std::unordered_map<std::string, Timer*>::iterator timer{ timers.begin() }; timer != timers.end(); ++timer)
		{
			delete (timer->second);
		}
		// map destructor will clear itself
	}



	//==================================================
	/* !
	@function  Timer::Timer
	@author    Isaiah Lim  (lim.i@digipen.edu)

	@params

	@brief	   Constructor, sets both start and end as the same time

	*//*__________________________________________________________________________*/
	Timer::Timer()
	{
		start_time = std::chrono::steady_clock::now();
		end_time = start_time;
	}

	//==================================================
	/* !
	@function  Timer::StartTime
	@author    Isaiah Lim  (lim.i@digipen.edu)

	@params

	@brief	   Start Timer

	*//*__________________________________________________________________________*/
	void Timer::StartTime()
	{
		start_time = std::chrono::steady_clock::now();
	}

	//==================================================
	/* !
	@function  Timer::EndTime
	@author    Isaiah Lim  (lim.i@digipen.edu)

	@params

	@brief	   End Timer

	*//*__________________________________________________________________________*/
	void Timer::EndTime()
	{
		end_time = std::chrono::steady_clock::now();
	}

	//==================================================
	/* !
	@function  Timer::GetTime
	@author    Isaiah Lim  (lim.i@digipen.edu)

	@params

	@brief	   Get time in milliseconds

	*//*__________________________________________________________________________*/
	const long long Timer::GetTime() const
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
	}

	void Timer::SetPercentage(float perc)
	{
		percentage = perc;
	}

	float Timer::GetPercentage() const
	{
		return percentage;
	}
}