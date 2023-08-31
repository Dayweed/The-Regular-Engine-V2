#pragma once
/*!
	@file    MemoryManager.hpp
	@author  Isaiah Lim (Code Contribution 100%)
	@email   lim.i@digipen.edu
	@date    14/09/2022
	@brief   This file handles all new and delete related to ecs and checks for
			 mem leaks

	Copyright (C) 2022 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/

/*                                                                 includes
----------------------------------------------------------------------------- */
#include "pch.h"
#include <map>
#include <string>
#include <iostream>

namespace TRE
{
	// Memory Manager [The ONLY ONE to handle new and delete]
	//==================================================
	struct MemoryManager // Handles Memory Allocation and should be run once at the start and end of main!
	{
	public:
		static MemoryManager& Instance();

		void AllocateSizes(size_t obj_size_, size_t comp_size_);

		// ECS Manager
		template <typename T>
		bool AllocateDerivedComponentOrSystem();
		bool AllocateObjectSize(size_t size_);
		bool AllocateComponentsAndSystemsSize(size_t size_);
		void DeleteAll();
		bool DeleteObjects();
		bool DeleteComponentsAndSystems();
		void ResetToConfig();

		// Set and Get
		void SetConfigObj(size_t config_obj_);
		size_t GetConfigObj() const;
		void SetConfigComp(size_t config_comp_);
		size_t GetConfigComp() const;

		// Size of each derived components/systems
		std::map<std::string, size_t> components_sizes{};

	private:
		// Delete possible copy ctor and assignment to ensure singleton
		MemoryManager() {};
		MemoryManager(MemoryManager const&) = delete;
		void operator=(MemoryManager const&) = delete;
		void* operator new(size_t) = delete;

		// Size set in config
		size_t config_obj{}, config_comp{};
	};
	static MemoryManager* _memory_manager{ &MemoryManager::Instance() };

	/* !
	@function	AllocateDerivedComponentOrSystem
	@author		Isaiah Lim Ji Rong  (lim.i@digipen.edu)

	@params		size_t size_ [This allocates the starting size of components]

	@brief		Allocates the components/systems by doubling it's original size

	*//*__________________________________________________________________________*/
	template <typename T>
	bool MemoryManager::AllocateDerivedComponentOrSystem()
	{
		// Update components
		if (components_sizes.find(typeid(T).name()) == components_sizes.end())
		{
			std::string type_name{ typeid(T).name() };
			LogWarn("[MemoryManager::AllocateDerivedComponentOrSystem]: Component [" + type_name + "] not found in components_sizes! Inserting in components_sizes...");
			components_sizes.insert({ typeid(T).name(), 0 });
		}

		// Resets and create components/systems based on size_
		size_t size{ (components_sizes[typeid(T).name()] == 0) ? 1 : components_sizes[typeid(T).name()] };
		for (size_t i{}; i < size; ++i)
		{
			T* comp = new T();
			(void)comp;
		}

		components_sizes[typeid(T).name()] += size;

		std::cout << "[MemoryManager::AllocateDerivedComponentOrSystem] Current size for component type [" << typeid(T).name() << "] is " << _memory_manager->components_sizes[typeid(T).name()] << "...\n";

		return true;
	}
}