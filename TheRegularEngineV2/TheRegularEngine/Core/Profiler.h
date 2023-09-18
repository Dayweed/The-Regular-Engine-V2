#pragma once
/*!
	@file    Profiler.hpp
	@author  Isaiah Lim (Code Contribution 100%)
	@email   lim.i@digipen.edu
	@date    30/08/2023
	@brief   This file contains the structs that handles the timing taken for each systems

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/

/*                                                                   includes
----------------------------------------------------------------------------- */
#include <chrono>

namespace TRE
{
	// Timer Records time
	struct Timer
	{
	public:
		Timer();
		void StartTime();
		void EndTime();
		const long long GetTime() const;

		void SetPercentage(float perc);
		float GetPercentage() const;

	private:
		std::chrono::steady_clock::time_point start_time{}, end_time{};
		float percentage{ 0.f };
	};

	// Profiler THERE CAN ONLY BE ONE! >:o
	//==================================================
	struct Profiler
	{
	public:
		static Profiler& Instance();

		// Timer creators
		void StartTimer(std::string name_);
		bool EndTimer(std::string name_);
		bool RemoveTimer(std::string name_);

		// Set Get Debugging Mode
		void SetDebugMode(bool mode_on_);
		bool GetDebugMode() const;

		// Debugging purposes for Profiler
		void PrintTimers();

		std::unordered_map<std::string, Timer*>& GetTimers();
		const long long GetTotalTime() const;
	private:
		// Ctor and Dtor
		Profiler();
		~Profiler();

		// Delete possible copy ctor and assignment to ensure singleton
		Profiler(Profiler const&) = delete;
		void operator=(Profiler const&) = delete;
		void* operator new(size_t) = delete;

		// Member Variables
		// Timers to store individual timings
		std::unordered_map<std::string, Timer*> timers;
		// Debug Mode
		bool debug_mode{ true };
		long long totalTime{ 0 };
		unsigned long long delay{ 1 };
		std::chrono::steady_clock::time_point start_delay{};
	};
}