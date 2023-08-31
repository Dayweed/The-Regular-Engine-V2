/*!
	@file    MemoryManager.cpp
	@author  Isaiah Lim (Code Contribution 100%)
	@email   lim.i@digipen.edu
	@date    14/09/2022
	@brief   This file handles all new and delete related to ecs and checks for
			 mem leaks

	Copyright (C) 2022 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/

#include "pch.h"
#include "MemoryManager.h"

//namespace TRE
//{
//	/* !
//	@function	Instance
//	@author		Isaiah Lim Ji Rong  (lim.i@digipen.edu)
//
//	@params
//
//	@brief		Creates a static instance of the MemoryManager
//
//	*//*__________________________________________________________________________*/
//	MemoryManager& MemoryManager::Instance()
//	{
//		static MemoryManager instance{};
//		return instance;
//	}
//
//	/* !
//	@function	AlocateSizes
//	@author		Isaiah Lim Ji Rong  (lim.i@digipen.edu)
//
//	@params		size_t obj_size_ [This allocates the starting size of objects]
//	@params		size_t comp_size_ [This allocates the starting size of components]
//
//	@brief		Allocates the starting sizes of objects and components
//
//	*//*__________________________________________________________________________*/
//	void MemoryManager::AllocateSizes(size_t obj_size_, size_t comp_size_)
//	{
//		AllocateComponentsAndSystemsSize(comp_size_);
//		AllocateObjectSize(obj_size_);
//	}
//
//	/* !
//	@function	AllocateObjectSize
//	@author		Isaiah Lim Ji Rong  (lim.i@digipen.edu)
//
//	@params		size_t size_ [Adds new amount of size_ into objects]
//
//	@brief		Adds the additional amount of objects into objects
//
//	*//*__________________________________________________________________________*/
//	bool MemoryManager::AllocateObjectSize(size_t size_)
//	{
//		// Resets and create objects based on size_
//		for (size_t i{}; i < size_; ++i)
//		{
//			GameObject* obj = new GameObject();
//			(void)obj;
//		}
//
//		// Successful Allocation
//		return true;
//	}
//
//	/* !
//	@function	AllocateComponentsAndSystemsSize
//	@author		Isaiah Lim Ji Rong  (lim.i@digipen.edu)
//
//	@params		size_t size_ [Adds new amount of size_ * different versions	]
//							 [of components/systems into components			]
//
//	@brief		Adds the additional amount of components/systems into components
//				NOTE! Every form of structs/classes derived from components/systems
//				must be added
//
//	*//*__________________________________________________________________________*/
//	bool MemoryManager::AllocateComponentsAndSystemsSize(size_t size_)
//	{
//		// Resets and create components based on size_
//		//// COMPONENTS
//		//for (size_t i{}; i < size_; ++i)
//		//{
//		//	Component* comp = new Component();
//		//	(void)comp;
//		//}
//
//		//// SYSTEM
//		//for (size_t i{}; i < size_; ++i)
//		//{
//		//	Component* comp = new System();
//		//	(void)comp;
//		//}
//
//		// SPRITERENDERER
//		for (size_t i{}; i < size_; ++i)
//		{
//			Component* comp = new SpriteRenderer();
//			(void)comp;
//		}
//		components_sizes.insert({ typeid(SpriteRenderer).name(), size_ });
//
//		// ANIMATOR
//		for (size_t i{}; i < size_; ++i)
//		{
//			Component* comp = new Animator();
//			(void)comp;
//		}
//		components_sizes.insert({ typeid(Animator).name(), size_ });
//
//		// ANIMATION CONTROLLER
//		for (size_t i{}; i < size_; ++i)
//		{
//			Component* comp = new AnimationController();
//			(void)comp;
//		}
//		components_sizes.insert({ typeid(AnimationController).name(), size_ });
//
//		// LINERENDERER
//		for (size_t i{}; i < size_; ++i)
//		{
//			Component* comp = new LineRenderer();
//			(void)comp;
//		}
//		components_sizes.insert({ typeid(LineRenderer).name(), size_ });
//
//		// TEXT RENDERER
//		for (size_t i{}; i < size_; ++i)
//		{
//			Component* comp = new Text();
//			(void)comp;
//		}
//		components_sizes.insert({ typeid(Text).name(), size_ });
//
//		// TRANSFORM
//		for (size_t i{}; i < size_; ++i)
//		{
//			Component* comp = new Transform();
//			(void)comp;
//		}
//		components_sizes.insert({ typeid(Transform).name(), size_ });
//
//		// KINEMATICS
//		for (size_t i{}; i < size_; ++i)
//		{
//			Component* comp = new Kinematics();
//			(void)comp;
//		}
//		components_sizes.insert({ typeid(Kinematics).name(), size_ });
//
//		// COLLISION
//		for (size_t i{}; i < size_; ++i)
//		{
//			Component* comp = new Collision();
//			(void)comp;
//		}
//		components_sizes.insert({ typeid(Collision).name(), size_ });
//
//		// CAMERA
//		for (size_t i{}; i < size_; ++i)
//		{
//			Component* comp = new Camera2D();
//			(void)comp;
//		}
//		components_sizes.insert({ typeid(Camera2D).name(), size_ });
//
//		// POINTCOLLIDER
//		for (size_t i{}; i < size_; ++i)
//		{
//			Component* comp = new PointCollider();
//			(void)comp;
//		}
//		components_sizes.insert({ typeid(PointCollider).name(), size_ });
//
//		// AABBCOLLIDER
//		for (size_t i{}; i < size_; ++i)
//		{
//			Component* comp = new AABBCollider();
//			(void)comp;
//		}
//		components_sizes.insert({ typeid(AABBCollider).name(), size_ });
//
//		// ARENACOLLIDER
//		for (size_t i{}; i < size_; ++i)
//		{
//			Component* comp = new ArenaCollider();
//			(void)comp;
//		}
//		components_sizes.insert({ typeid(ArenaCollider).name(), size_ });
//
//		// AUDIO
//		for (size_t i{}; i < size_; ++i)
//		{
//			Component* comp = new Audio();
//			(void)comp;
//		}
//		components_sizes.insert({ typeid(Audio).name(), size_ });
//
//		// DIALOGUE SYSTEM
//		for (size_t i{}; i < size_; ++i)
//		{
//			Component* comp = new Dialogue();
//			(void)comp;
//		}
//		components_sizes.insert({ typeid(Dialogue).name(), size_ });
//
//		// Combat
//		for (size_t i{}; i < size_; ++i)
//		{
//			Component* comp = new CombatBehaviour();
//			(void)comp;
//		}
//		components_sizes.insert({ typeid(CombatBehaviour).name(), size_ });
//
//		// SCRIPT
//		for (size_t i{}; i < size_; ++i)
//		{
//			Component* comp = new Script();
//			(void)comp;
//		}
//		components_sizes.insert({ typeid(Script).name(), size_ });
//
//		// PARTICLE SYSTEM
//		for (size_t i{}; i < size_; ++i)
//		{
//			Component* comp = new ParticleSystem();
//			(void)comp;
//		}
//		components_sizes.insert({ typeid(ParticleSystem).name(), size_ });
//
//		// PARTICLE
//		for (size_t i{}; i < size_; ++i)
//		{
//			Component* comp = new Particle();
//			(void)comp;
//		}
//		components_sizes.insert({ typeid(Particle).name(), size_ });
//
//		// The following components below is meant to be converted to Lua in the next milestone!
//		// =====================================================================================
//		// BUTTON
//		for (size_t i{}; i < size_; ++i)
//		{
//			Component* comp = new Button();
//			(void)comp;
//		}
//		components_sizes.insert({ typeid(Button).name(), size_ });
//
//		// Successful Allocation
//		return true;
//	}
//
//	/* !
//	@function	DeleteAll
//	@author		Isaiah Lim Ji Rong  (lim.i@digipen.edu)
//
//	@params
//
//	@brief		Deletes all objects, components and systems
//
//	*//*__________________________________________________________________________*/
//	void MemoryManager::DeleteAll()
//	{
//		DeleteComponentsAndSystems();
//		DeleteObjects();
//	}
//
//	/* !
//	@function	DeleteObjects
//	@author		Isaiah Lim Ji Rong  (lim.i@digipen.edu)
//
//	@params
//
//	@brief		Deletes all objects in objects
//
//	*//*__________________________________________________________________________*/
//	bool MemoryManager::DeleteObjects()
//	{
//		for (GameObject* object : _ecs_manager->objects)
//		{
//			delete object;
//			object = NULL;
//		}
//		_ecs_manager->objects.clear();
//		_ecs_manager->undeployed_objects.clear();
//		_ecs_manager->deployed_objects.clear();
//
//		// Successful deletion
//		return true;
//	}
//
//	/* !
//	@function	DeleteComponentsAndSystems
//	@author		Isaiah Lim Ji Rong  (lim.i@digipen.edu)
//
//	@params
//
//	@brief		Deletes all components and systems in components
//
//	*//*__________________________________________________________________________*/
//	bool MemoryManager::DeleteComponentsAndSystems()
//	{
//		for (Component* comp : _ecs_manager->components)
//		{
//			delete comp;
//			comp = NULL;
//		}
//		_ecs_manager->components.clear();
//		_ecs_manager->undeployed_components.clear();
//		_ecs_manager->deployed_components.clear();
//		_ecs_manager->systems.clear();
//		_ecs_manager->undeployed_systems.clear();
//		_ecs_manager->deployed_systems.clear();
//
//		// Successful deletion
//		return true;
//	}
//
//	/* !
//	@function	MemoryManager::ResetToConfig
//	@author		Isaiah Lim Ji Rong  (lim.i@digipen.edu)
//
//	@params
//
//	@brief		Deletes all undeployed objects and components, leaving only
//				x amount of objects/components (Deployed or Undeployed) available
//				based on the config_obj & config_comp
//
//				This means it is possible that there are more deployed objects
//				or components than the config size
//
//	*//*__________________________________________________________________________*/
//	void MemoryManager::ResetToConfig()
//	{
//		for (int i{ static_cast<int>(_ecs_manager->objects.size()) - 1 }; i >= 0; --i)
//		{
//			// Delete if objects is undeployed and there are more existing objects than the config_obj
//			if (!_ecs_manager->objects[i]->GetDeployed() && _ecs_manager->objects.size() > config_obj)
//			{
//				// Remove from undeployed_objects
//				if (std::find(_ecs_manager->undeployed_objects.begin(), _ecs_manager->undeployed_objects.end(), _ecs_manager->objects[i]) != _ecs_manager->undeployed_objects.end())
//				{
//					_ecs_manager->undeployed_objects.erase(std::find(_ecs_manager->undeployed_objects.begin(), _ecs_manager->undeployed_objects.end(), _ecs_manager->objects[i]));
//				}
//				// Remove from deployed_objects
//				if (std::find(_ecs_manager->deployed_objects.begin(), _ecs_manager->deployed_objects.end(), _ecs_manager->objects[i]) != _ecs_manager->deployed_objects.end())
//				{
//					_ecs_manager->deployed_objects.erase(std::find(_ecs_manager->deployed_objects.begin(), _ecs_manager->deployed_objects.end(), _ecs_manager->objects[i]));
//					std::cout << "MemoryManager::ResetToConfig(): (Object) This should NEVER happen\n";
//				}
//				delete _ecs_manager->objects[i];
//				_ecs_manager->objects.erase(_ecs_manager->objects.begin() + i);
//			}
//		}
//
//		// COMPONENTS
//		for (int i{ static_cast<int>(_ecs_manager->components.size()) - 1 }; i >= 0; --i)
//		{
//			//std::cout << _ecs_manager->components[i]->GetType() << "\n";
//			// Delete if components is undeployed and there are more existing components than the config_obj
//			/*if (_memory_manager->components_sizes.find(typeid(_ecs_manager->components[i]).name()) != _memory_manager->components_sizes.end())
//				std::cout << "[1]: " << !_ecs_manager->components[i]->GetDeployed() << " [2]: " << typeid(_ecs_manager->components[i]).name() << " [3]: " << _memory_manager->components_sizes[typeid(_ecs_manager->components[i]).name()] << "\n";*/
//			if (!_ecs_manager->components[i]->GetDeployed() && _memory_manager->components_sizes.find(_ecs_manager->components[i]->GetType()) != _memory_manager->components_sizes.end()
//				&& _memory_manager->components_sizes[_ecs_manager->components[i]->GetType()] > config_comp)
//			{
//				// Delete it also in systems
//				std::vector<System*>::iterator it{ std::find(_ecs_manager->systems.begin(), _ecs_manager->systems.end(), _ecs_manager->components[i]) };
//				if (it != _ecs_manager->systems.end())
//				{
//					_ecs_manager->systems.erase(it);
//					// Remove from undeployed_systems
//					if (std::find(_ecs_manager->undeployed_systems.begin(), _ecs_manager->undeployed_systems.end(), _ecs_manager->components[i]) != _ecs_manager->undeployed_systems.end())
//					{
//						_ecs_manager->undeployed_systems.erase(std::find(_ecs_manager->undeployed_systems.begin(), _ecs_manager->undeployed_systems.end(), _ecs_manager->components[i]));
//					}
//					// Remove from deployed_systems
//					if (std::find(_ecs_manager->deployed_systems.begin(), _ecs_manager->deployed_systems.end(), _ecs_manager->components[i]) != _ecs_manager->deployed_systems.end())
//					{
//						_ecs_manager->deployed_systems.erase(std::find(_ecs_manager->deployed_systems.begin(), _ecs_manager->deployed_systems.end(), _ecs_manager->components[i]));
//						LogWarn("MemoryManager::ResetToConfig(): (Object) This should NEVER happen");
//					}
//				}
//
//
//				// Remove from undeployed_components
//				if (std::find(_ecs_manager->undeployed_components.begin(), _ecs_manager->undeployed_components.end(), _ecs_manager->components[i]) != _ecs_manager->undeployed_components.end())
//				{
//					_ecs_manager->undeployed_components.erase(std::find(_ecs_manager->undeployed_components.begin(), _ecs_manager->undeployed_components.end(), _ecs_manager->components[i]));
//				}
//				// Remove from deployed_components
//				if (std::find(_ecs_manager->deployed_components.begin(), _ecs_manager->deployed_components.end(), _ecs_manager->components[i]) != _ecs_manager->deployed_components.end())
//				{
//					_ecs_manager->deployed_components.erase(std::find(_ecs_manager->deployed_components.begin(), _ecs_manager->deployed_components.end(), _ecs_manager->components[i]));
//					LogWarn("MemoryManager::ResetToConfig(): (Object) This should NEVER happen");
//				}
//				--_memory_manager->components_sizes[_ecs_manager->components[i]->GetType()];
//				delete _ecs_manager->components[i];
//				_ecs_manager->components.erase(_ecs_manager->components.begin() + i);
//			}
//		}
//	}
//
//	void MemoryManager::SetConfigObj(size_t config_obj_)
//	{
//		config_obj = config_obj_;
//	}
//
//	size_t MemoryManager::GetConfigObj() const
//	{
//		return config_obj;
//	}
//
//	void MemoryManager::SetConfigComp(size_t config_comp_)
//	{
//		config_comp = config_comp_;
//	}
//
//	size_t MemoryManager::GetConfigComp() const
//	{
//		return config_comp;
//	}
//}