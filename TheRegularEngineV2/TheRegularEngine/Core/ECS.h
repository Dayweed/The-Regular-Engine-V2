#pragma once
#include "pch.h"
#include "entt.hpp"
#include "System.h"
#include "ComponentManager.h"
#include "Transform.h"
#include <typeindex>
#include "Core/Logger.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"

namespace TRE
{
	// DO NOT USE THIS UNLESS YOU WANT THE Entity TO BE DELETED!
	// Get this component in GetEntities to get Entity that are going to be deleted in this loop
	class Removal
	{
		//static constexpr auto in_place_delete = false;
		bool fake; //This value is to ensure it can compile and be registered
	};
	class Undeployed
	{
		bool fake;
	};

	class Properties
	{
	public:
		std::string m_Name; // To get the name
		bool m_Active;		// To check if it is active
	};

	class Ent;
	typedef std::shared_ptr<Ent> Entity;

	typedef std::uint32_t Entity_ID;

	class Ent : public std::enable_shared_from_this<Ent>
	{
	public:
		/* !
		@function	GetThis
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Returns this in the form of a shared_ptr (Entity)
					Note! Not using Ent, but using the shared_ptr variant.
					This should replace the variable this
		*//*__________________________________________________________________________*/
		Entity GetThis();

		/* !
		@function	HasComponent
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Checks if this Entity has a specific component

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		if (goVar->HasComponent<ComponentStruct>())
		{
			// Use Properties Component
		}
		*//*__________________________________________________________________________*/
		template <typename T>
		bool HasComponent();

		/* !
		@function	AddComponent
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Adds the component into the Entity
					If it already exist, can be used an alternative GetComponent

		[Warning]	If the typename is not registered in _component_manager, it will
					cause an assert

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		goVar->AddComponent<ComponentStruct>();

		goVar->AddComponent<ComponentStruct>().Var = 0;
		*//*__________________________________________________________________________*/
		template <typename T>
		T& AddComponent();

		/* !
		@function	GetComponent
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Get the component from the Entity

		[Warning]	If the typename is not registered in _component_manager,
					or the Entity is deleted or the Entity does not have the component,
					it will	cause an assert

		Example:
		if 
		*//*__________________________________________________________________________*/
		template <typename T>
		T& GetComponent();

		/* !
		@function	RemoveComponent
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Remove the component from the Entity

		[Warning]	If the typename is not registered in _component_manager, it will
					cause an assert

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		if (goVar->HasComponent<ComponentStruct>())
		{
			goVar->GetComponent<ComponentStruct>().Val = 0;
		}
		*//*__________________________________________________________________________*/
		template <typename T>
		void RemoveComponent();

		/* !
		@function	SetParent
		@author		Isaiah Lim lim.i@digipen.edu

		@params		Entity parent // Entity for the new parent

		@brief		Abandons the previous parent if it exist
					Set the parent of the Entity
					Automatically add this Entity to the parent's children list

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);
		*//*__________________________________________________________________________*/
		void SetParent(Entity parent);

		/* !
		@function	GetParent
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Returns the parent of the Entity
					Returns nullptr if it doesn't exist

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);

		Entity AccessEntityVarParent = goVar->GetParent();
		*//*__________________________________________________________________________*/
		Entity GetParent();

		/* !
		@function	RemoveParent
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Remove the parent of the Entity
					Automatically Abandons this Entity from the parent's children list

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);

		goVar->RemoveParent();
		*//*__________________________________________________________________________*/
		void RemoveParent();

		/* !
		@function	AddChild
		@author		Isaiah Lim lim.i@digipen.edu

		@params		Entity child // Entity for the child

		@brief		Add a child to this Entity children list
					Automatically set this Entity as the parent of the child

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goChildVar = ECSManager::Instance().CreateEntity("goChildVar");

		goVar->AddChild(goChildVar);
		*//*__________________________________________________________________________*/
		void AddChild(Entity child);

		/* !
		@function	GetChildren
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Returns the children of the Entity

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);

		std::vector<Entity> goParentVarChildren = goParentVar->GetChildren();
		*//*__________________________________________________________________________*/
		std::vector<Entity> GetChildren();

		/* !
		@function	AbandonChild
		@author		Isaiah Lim lim.i@digipen.edu

		@params		Entity child // Entity for the child

		@brief		Remove child from children list
					Automatically remove parent from the child Entity
					Ignores command if child's parent is not this Entity

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);

		goParentVar->AbandonChild(goVar);
		*//*__________________________________________________________________________*/
		void AbandonChild(Entity child);

		/* !
		@function	AbandonChildren
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Remove all children from children list
					Automatically remove parent from each child Entity

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);

		goParentVar->AbandonChildren();
		*//*__________________________________________________________________________*/
		void AbandonChildren();
		
	private:
		friend class ECSManager;
		friend class MemoryManager;

		Entity m_Parent;
		std::vector<Entity> m_Children;

		entt::entity m_Entity;
	};

	class ECSOutputArchive
	{
	public:
		ECSOutputArchive(std::string filePath);
		void operator()(entt::entity ent);
		void operator()(std::underlying_type_t<entt::entity> u);
		template <typename T>
		void operator()(entt::entity ent, const T& t)
		{
			//rapidjson::Value Array(rapidjson::kObjectType);

			m_Doc.PushBack(static_cast<uint32_t>(ent), m_Doc.GetAllocator());
			//rapidjson::Value v = t;
			//Array.PushBack(v, m_Doc.GetAllocator());

			std::cout << "&";
		}
		void Close();
	private:
		std::string m_FilePath;
		rapidjson::Document m_Doc;
	};

	class ECSInputArchive
	{
	public:
		ECSInputArchive(std::string filePath);
		void operator()(entt::entity& ent);
		void operator()(std::underlying_type_t<entt::entity>& u);
		template <typename T>
		void operator()(entt::entity ent, const T& t)
		{
			std::cout << "\\/";
		}
	private:
		std::string m_FilePath;
		rapidjson::Document m_Doc;
	};

	// ECS Manager (Entity Manager) THERE CAN ONLY BE ONE! >:o
	//==================================================
	class ECSManager
	{
	public:
		/* !
		@function	Instance
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Holds the singleton for the ECSManager
					This shouldn't be used, use ECSManager::Instance() instead
		*//*__________________________________________________________________________*/
		static ECSManager& Instance();

		/* !
		@function	GetRegistry
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Returns entt registry in the ECSManager::Instance() singleton
					This should ideally be used for ECSManager::Instance() only

		Example:
		ECSManager::Instance().GetRegistry();
		*//*__________________________________________________________________________*/
		entt::registry& GetRegistry();

		// Engine Loop
		/* !
		@function	DestroyRemovalEntities
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Delete all Entity that has the component Removal
					This is only run under Engine.cpp at the end of the frame

		Example:
		ECSManager::Instance().DestroyRemovalEntity();
		*//*__________________________________________________________________________*/
		void DeleteRemovalEntities();

		// Shutdown Functions
		/* !
		@function	DestroyAll
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Destroys all the Entitys

		Example:
		ECSManager::Instance().DestroyAll();
		*//*__________________________________________________________________________*/
		void DestroyAll();

		// Entity Controller
		/* !
		@function	CreateEntity
		@author		Isaiah Lim lim.i@digipen.edu

		@params		std::string name	// Default name is "Ent", can be renamed
										// name is found in Properties::m_Name

		@brief		Create a new Entity
					Automatically adds the Properties Component

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");

		std::cout << goVar->HasComponent<Properties>() << std::endl;
		std::cout << goVar->GetComponent<Properties>().m_Name << std::endl;
		// Output
		// true
		// goVar
		*//*__________________________________________________________________________*/
		Entity CreateEntity(std::string name = "Ent");

		/* !
		@function	MarkForDeletion
		@author		Isaiah Lim lim.i@digipen.edu

		@params		Entity& object			// Entity to be destroyed

		@brief		Create a new Entity

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");

		std::cout << goVar << std::endl; // Address of goVar

		ECSManager::Instance().MarkForDeletion(goVar);
		*//*__________________________________________________________________________*/
		void MarkForDeletion(Entity& object);

		/* !
		@function	CloneEntity
		@author		Isaiah Lim lim.i@digipen.edu

		@params		Entity& object			// Entity to clone from
					std::string name	// Name for the cloned Entity

		@brief		Create a new Entity from an existing Entity

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity clonGoVar = ECSManager::Instance().CloneEntity(goVar, "ClonedEntityName");
		*//*__________________________________________________________________________*/
		Entity CloneEntity(Entity& object, std::string name = "Cloned_GameObject");

		/* !
		@function	EntityHasComponent
		@author		Isaiah Lim lim.i@digipen.edu

		@params		Entity& object			// Entity to check from

		@brief		Check if the Entity have a component

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		goVar->AddComponent<Transform>();

		std::cout << ECSManager::Instance().EntityHasComponent<Transform>(goVar) << std::endl;
		std::cout << ECSManager::Instance().EntityHasComponent<HUMAN>(goVar) << std::endl;

		// Output
		// true
		// false
		*//*__________________________________________________________________________*/
		template <typename T>
		bool EntityHasComponent(Entity object);

		/* !
		@function	GetEntities
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Returns a vector of Entity with the components listed

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("Object1");
		goVar->AddComponent<Transform>();
		goVar->AddComponent<Component>();

		Entity goVar2 = ECSManager::Instance().CreateEntity("Object2");

		Entity goVar3 = ECSManager::Instance().CreateEntity("Object3");
		goVar3->AddComponent<Component>();
		ECSManager::Instance().DestroyEntity(goVar3);

		std::cout << "Entity with Properties Size: " << ECSManager::Instance().GetEntities<Properties>().size() << std::endl;
		for (Entity& obj : ECSManager::Instance().GetEntities<Properties>())
			std::cout << "- " << obj->GetComponent<Properties>().m_Name << std::endl;

		std::cout << "Entity with Transform Size: " << ECSManager::Instance().GetEntities<Transform>().size() << std::endl;
		for (Entity& obj : ECSManager::Instance().GetEntities<Transform>())
			std::cout << "- " << obj->GetComponent<Properties>().m_Name << std::endl;

		std::cout << "Entity with Component Size: " << ECSManager::Instance().GetEntities<Component>().size() << std::endl;
		for (Entity& obj : ECSManager::Instance().GetEntities<Component>())
			std::cout << "- " << obj->GetComponent<Properties>().m_Name << std::endl;

		std::cout << "Entity with Removal Size: " << ECSManager::Instance().GetEntities<Removal>().size() << std::endl;
		for (Entity& obj : ECSManager::Instance().GetEntities<Removal>())
			std::cout << "- " << obj->GetComponent<Properties>().m_Name << std::endl;

		std::cout << "Entity with Transform and Component Size: " << ECSManager::Instance().GetEntities<Transform, Component>().size() << std::endl;
		for (Entity& obj : ECSManager::Instance().GetEntities<Transform, Component>())
			std::cout << "- " << obj->GetComponent<Properties>().m_Name << std::endl;

		// Output
		// Entity with Properties Size: 3
		// - Object1
		// - Object2
		// - Object3
		// Entity with Transform Size: 3
		// - Object1
		// - Object2
		// - Object3
		// Entity with Component Size: 2
		// - Object1
		// - Object3
		// Entity with Removal Size: 1
		// - Object3
		// Entity with Transform and Component and Removal Size: 1
		// - Object3
		*//*__________________________________________________________________________*/
		template <typename Comp, typename... Others>
		std::vector<Entity> GetEntities();

		/* !
		@function	GetAllEntities
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Returns a vector of all Entities
		*//*__________________________________________________________________________*/
		std::vector<Entity> GetAllEntities();

		void SaveEntities(std::string filePath);

		void LoadEntities(std::string filePath);

		// TODELETE
		void TESTRUN();

	private:
		friend class MemoryManager;

		// Delete possible copy ctor and assignment to ensure singleton
		ECSManager() {};
		ECSManager(ECSManager const&) = delete;
		void operator=(ECSManager const&) = delete;
		void* operator new(size_t) = delete;

		// EnTT stuff
		entt::registry registry;

		// Component Types
		std::map<size_t, void*> componentTypes;

		std::unordered_map<Entity_ID, Entity> m_EntityList;
	};


	template <typename Comp, typename... Others>
	std::vector<Entity> ECSManager::GetEntities()
	{
		std::vector<Entity> objects{};
		entt::exclude_t<Undeployed> u{};
		auto view = registry.view<Comp, Others...>(u);
		objects.reserve(m_EntityList.size());

		// Get all Entity owning the entities
		for (entt::entity obj : view)
		{
			if (m_EntityList.find(static_cast<Entity_ID>(obj)) != m_EntityList.end())
			{
				objects.emplace_back(m_EntityList[static_cast<Entity_ID>(obj)]);
			}
		}

		return objects;
	}

	template <typename T>
	bool ECSManager::EntityHasComponent(Entity object)
	{
		return registry.any_of<T>(object->m_Entity);
	}

	template <typename T>
	bool Ent::HasComponent()
	{
		if (!ComponentManager::Instance().HasComponent<T>() && !ComponentManager::Instance().HasHiddenComponent<T>())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			TRE_CORE_ERROR("[" + funcName + "] Component " + compName + " is not included in _component_manager");
			assert(ComponentManager::Instance().HasComponent<T>() || ComponentManager::Instance().HasHiddenComponent<T>());
		}
		return ECSManager::Instance().EntityHasComponent<T>(shared_from_this());
	}

	template <typename T>
	T& Ent::AddComponent()
	{
		if (!ComponentManager::Instance().HasComponent<T>() && !ComponentManager::Instance().HasHiddenComponent<T>())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			TRE_CORE_ERROR("[" + funcName + "] Component " + compName + " is not included in _component_manager");
			assert(ComponentManager::Instance().HasComponent<T>() || ComponentManager::Instance().HasHiddenComponent<T>());
		}

		if (HasComponent<T>())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			TRE_CORE_ERROR("[" + funcName + "] Component " + compName + " is already in " + GetComponent<Properties>().m_Name + "...");
			return GetComponent<T>();
		}
		return ECSManager::Instance().GetRegistry().emplace<T>(m_Entity);
	}

	template <typename T>
	T& Ent::GetComponent()
	{
		// Ensure cannot get a component from a freed object and entity
		if (this == nullptr || &m_Entity == nullptr)
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] " + GetComponent<Properties>().m_Name + " is no longer valid (this or entity is nullptr)");
			assert(this != nullptr);
			assert(&m_Entity != nullptr);
		}

		if (!ComponentManager::Instance().HasComponent<T>() && !ComponentManager::Instance().HasHiddenComponent<T>())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			TRE_CORE_ERROR("[" + funcName + "] Component " + compName + " is not included in _component_manager");
			assert(ComponentManager::Instance().HasComponent<T>() || ComponentManager::Instance().HasHiddenComponent<T>());
		}

		if (!HasComponent<T>())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			TRE_CORE_ERROR("[" + funcName + "] " + GetComponent<Properties>().m_Name + " does not have the component " + compName);
			assert(HasComponent<T>());
		}

		return ECSManager::Instance().GetRegistry().get<T>(m_Entity);
	}

	template <typename T>
	void Ent::RemoveComponent()
	{
		// Ensure cannot get a component from a freed object and entity
		if (this == nullptr || &m_Entity == nullptr)
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] " + GetComponent<Properties>().m_Name + " is no longer valid (this or entity is nullptr)");
			assert(this != nullptr);
			assert(&m_Entity != nullptr);
		}

		if (!ComponentManager::Instance().HasComponent<T>() && !ComponentManager::Instance().HasHiddenComponent<T>())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			TRE_CORE_ERROR("[" + funcName + "] Component " + compName + " is not included in _component_manager");
			assert(ComponentManager::Instance().HasComponent<T>() || ComponentManager::Instance().HasHiddenComponent<T>());
		}

		if (HasComponent<T>())
			ECSManager::Instance().GetRegistry().remove<T>(m_Entity);
	}
}