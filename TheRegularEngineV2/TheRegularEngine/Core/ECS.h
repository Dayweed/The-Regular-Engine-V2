#pragma once
/*!
	@file		ECS.h
	@author		Isaiah Lim (Code Contribution 100%)
	@email		lim.i@digipen.edu
	@date		25/08/2023
	@brief		Handles all the front end of handling entities and linking
				with the entt::entities

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/

/*                                                                 includes
----------------------------------------------------------------------------- */
#include "pch.h"
#include "entt.hpp"
#include "System.h"
#include "ComponentManager.h"
#include "Transform.h"
#include <typeindex>
#include "Core/Logger.h"

#include <nlohmann/json.hpp>

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"

/*                                                                 defines
----------------------------------------------------------------------------- */
#define ECS_ENTITY_DEFAULT_NAME "Default_Entity_Name"
#define ECS_CLONE_DEFAULT_NAME "Clone_Entity_Name"

namespace TRE
{
	class Ent;
	typedef std::shared_ptr<Ent> Entity;

	typedef std::uint32_t Entity_ID;

	// DO NOT USE THIS UNLESS YOU WANT THE Entity TO BE DELETED!
	// Get this component in GetEntities to get Entity that are going to be deleted in this loop
	class Removal
	{
		//static constexpr auto in_place_delete = false;
		bool m_Fake; //This value is to ensure it can compile and be registered
	};
	// DO NOT USE THIS UNLESS IT IS CREATING ALLOCATED ENTITIES!
	// Get this component in GetEntities to get Entity that are going to be deleted in this loop
	class Undeployed
	{
		bool m_Fake; //This value is to ensure it can compile and be registered
	};

	struct Properties
	{
		std::string m_GUID{};
		bool m_Active{ true };		// To check if it is active
		std::string m_Name{};		// To get the name

		Properties() = default;
		~Properties() = default;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Properties, m_Active, m_GUID, m_Name)
	};

	struct Parenting
	{
		std::string m_Parent{};
		std::vector<std::string> m_Children{};

		Parenting() = default;
		~Parenting() = default;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Parenting, m_Parent, m_Children)
	};

	class Ent : public std::enable_shared_from_this<Ent>
	{
	public:
		/* !
		@function		GetThis
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Returns this in the form of a shared_ptr (Entity)
						Note! Not using Ent, but using the shared_ptr variant.
						This should replace the variable this
		*//*__________________________________________________________________________*/
		Entity GetThis();

		/* !
		@function		HasComponent
		@author			Isaiah Lim lim.i@digipen.edu

		@brief			Checks if this Entity has a specific component

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
		@function		AddComponent
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Adds the component into the Entity
						If it already exist, can be used an alternative GetComponent

		[Warning]		If the typename is not registered in _component_manager, it will
						cause an assert

		Example:
		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		goVar->AddComponent<ComponentStruct>();

		goVar->AddComponent<ComponentStruct>().Var = 0;
		*//*__________________________________________________________________________*/
		template <typename T>
		T& AddComponent();

		/* !
		@function		GetComponent
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Get the component from the Entity

		[Warning]		If the typename is not registered in _component_manager,
						or the Entity is deleted or the Entity does not have the component,
						it will	cause an assert

		Example:
		if 
		*//*__________________________________________________________________________*/
		template <typename T>
		T& GetComponent();

		/* !
		@function		RemoveComponent
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Remove the component from the Entity

		[Warning]		If the typename is not registered in _component_manager, it will
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
		@function		SetParent
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			parent	Entity for the new parent

		@brief			Abandons the previous parent if it exist
						Set the parent of the Entity
						Automatically add this Entity to the parent's children list

		Example:
		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);
		*//*__________________________________________________________________________*/
		void SetParent(Entity parent);

		/* !
		@function		GetParent
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Returns the parent of the Entity
						Returns nullptr if it doesn't exist

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);

		Entity AccessEntityVarParent = goVar->GetParent();
		*//*__________________________________________________________________________*/
		Entity GetParent();

		/* !
		@function		RemoveParent
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Remove the parent of the Entity
						Automatically Abandons this Entity from the parent's children list

		Example:
		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);

		goVar->RemoveParent();
		*//*__________________________________________________________________________*/
		void RemoveParent();

		/* !
		@function		AddChild
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			child	Entity for the child

		@brief			Add a child to this Entity children list
						Automatically set this Entity as the parent of the child

		Example:
		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goChildVar = ECSManager::Instance().CreateEntity("goChildVar");

		goVar->AddChild(goChildVar);
		*//*__________________________________________________________________________*/
		void AddChild(Entity child);

		/* !
		@function		GetChildren
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Returns the vector of children of the Entity

		Example:
		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);

		std::vector<Entity> goParentVarChildren = goParentVar->GetChildren();
		*//*__________________________________________________________________________*/
		std::vector<Entity> GetChildren();

		/* !
		@function		AbandonChild
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			child	Entity for the child

		@brief			Remove child from children list
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
		@function		AbandonChildren
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Remove all children from children list
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

		entt::entity m_Entity;
	};

	// ECS Manager (Entity Manager) THERE CAN ONLY BE ONE! >:o
	//==================================================
	class ECSManager
	{
	public:
		/* !
		@function		Instance
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Holds the singleton for the ECSManager
						This shouldn't be used, use ECSManager::Instance() instead
		*//*__________________________________________________________________________*/
		static ECSManager& Instance();

		/* !
		@function		GetRegistry
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Returns entt registry in the ECSManager::Instance() singleton
						This should ideally be used for ECSManager::Instance() only

		Example:
		ECSManager::Instance().GetRegistry();
		*//*__________________________________________________________________________*/
		entt::registry& GetRegistry();

		// Engine Loop
		/* !
		@function		DestroyRemovalEntities
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Delete all Entity that has the component Removal
						This is only run under Engine.cpp at the end of the frame

		Example:
		ECSManager::Instance().DestroyRemovalEntity();
		*//*__________________________________________________________________________*/
		void DeleteRemovalEntities();

		// Shutdown Functions
		/* !
		@function		DestroyAll
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Destroys all the Entitys

		Example:
		ECSManager::Instance().DestroyAll();
		*//*__________________________________________________________________________*/
		void DestroyAll();

		// Entity Controller
		/* !
		@function		CreateEntity
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			name	Default name is "Ent", can be renamed
								name is found in Properties::m_Name

		@brief			Create a new Entity
						Automatically adds the Properties Component

		Example:
		Entity goVar = ECSManager::Instance().CreateEntity("goVar");

		std::cout << goVar->HasComponent<Properties>() << std::endl;
		std::cout << goVar->GetComponent<Properties>().m_Name << std::endl;
		
		Output:
		true
		goVar
		*//*__________________________________________________________________________*/
		Entity CreateEntity(std::string name = ECS_ENTITY_DEFAULT_NAME);

		/* !
		@function		MarkForDeletion
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			object	Entity to be destroyed

		@brief			Create a new Entity

		Example:
		Entity goVar = ECSManager::Instance().CreateEntity("goVar");

		std::cout << goVar << std::endl; // Address of goVar

		ECSManager::Instance().MarkForDeletion(goVar);
		*//*__________________________________________________________________________*/
		void MarkForDeletion(Entity& object);

		/* !
		@function		CloneEntity
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			object	Entity to clone from
						name	Name for the cloned Entity

		@brief			Create a new Entity from an existing Entity

		Example:
		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity clonGoVar = ECSManager::Instance().CloneEntity(goVar, "ClonedEntityName");
		*//*__________________________________________________________________________*/
		Entity CloneEntity(Entity& object, std::string name = ECS_CLONE_DEFAULT_NAME);

		/* !
		@function		EntityHasComponent
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			object	Entity to check from

		@brief			Check if the Entity have a component

		Example:
		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		goVar->AddComponent<Transform>();

		std::cout << ECSManager::Instance().EntityHasComponent<Transform>(goVar) << std::endl;
		std::cout << ECSManager::Instance().EntityHasComponent<HUMAN>(goVar) << std::endl;

		Output:
		true
		false
		*//*__________________________________________________________________________*/
		template <typename T>
		bool EntityHasComponent(Entity object);

		/* !
		@function		GetEntities
		@author			Isaiah Lim lim.i@digipen.edu

		@brief			Returns a vector of Entity with the components listed

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

		Output:
		Entity with Properties Size: 3
		- Object1
		- Object2
		- Object3
		Entity with Transform Size: 3
		- Object1
		- Object2
		- Object3
		Entity with Component Size: 2
		- Object1
		- Object3
		Entity with Removal Size: 1
		- Object3
		Entity with Transform and Component and Removal Size: 1
		- Object3
		*//*__________________________________________________________________________*/
		template <typename Comp, typename... Others>
		std::vector<Entity> GetEntities();

		/* !
		@function		GetAllEntities
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Returns a vector of all Entities
		*//*__________________________________________________________________________*/
		std::vector<Entity> GetAllEntities();

		/* !
		@function		SaveEntities
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			filePath	File Name, including .json

		@brief			Saves Entities in a json file

		Example:
		std::cout << ECSManager::Instance().SaveEntities("TestFile.json") << "\n";

		Output:
		../Scenes/TestFile.json
		*//*__________________________________________________________________________*/
		std::string SaveEntities(std::string filePath);

		/* !
		@function		LoadEntities
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			filePath	Entire File Path Name, including .json

		@brief			Loads Entities from a json file

		Example:
		ECSManager::Instance().LoadEntities("TestFile.json");
		*//*__________________________________________________________________________*/
		void LoadEntities(std::string filePath);

		/* !
		@function		FindEntity
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			id	string of GUID of the Entity

		@brief			Returns the Entity based on the GUID
						Returns a nullptr if GUID is invalid

		Example:
		Entity entitySample = CreateEntity("entitySample");
		std::string entityGUID = entitySample->GetComponent<Properties>().m_GUID;

		std::cout << "1) " << ECSManager::Instance().FindEntity(entityGUID) << "\n";

		// This would not delete the object!
		ECSManager::Instance().MarkForDeletion(entitySample);
		std::cout << "2) " << ECSManager::Instance().FindEntity(entityGUID) << "\n";

		// Destroy All deletes all the entities immediately (This is for debugging mostly)
		ECSManager::Instance().DestroyAll();
		std::cout << "3) " << ECSManager::Instance().FindEntity(entityGUID) << "\n";

		Output:
		1) 00000204471513A0
		2) 00000204471513A0
		3) 0000000000000000
		*//*__________________________________________________________________________*/
		Entity FindEntity(std::string id);

		/* !
		@function		FindEntityID
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			ent	Entity to find the GUID string

		@brief			Returns the GUID based on the entity
						Returns a "" if entity is invalid

		Example:
		Entity entitySample = CreateEntity("entitySample");

		std::cout << "1) " << ECSManager::Instance().FindEntityID(entitySample) << "\n";

		// This would not delete the object!
		ECSManager::Instance().MarkForDeletion(entitySample);
		std::cout << "2) " << ECSManager::Instance().FindEntityID(entitySample) << "\n";

		// Destroy All deletes all the entities immediately (This is for debugging mostly)
		ECSManager::Instance().DestroyAll();
		std::cout << "3) " << ECSManager::Instance().FindEntityID(entitySample) << "\n";

		Output:
		1) "{03A691F2-02DB-4957-8A0E-CA2BDAB9CDF8}"
		2) "{03A691F2-02DB-4957-8A0E-CA2BDAB9CDF8}"
		3) ""
		*//*__________________________________________________________________________*/
		std::string FindEntityID(Entity ent);

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
		entt::registry m_Registry;

		std::unordered_map<std::string, Entity> m_EntityList;
	};

	class ECSOutputArchive
	{
	public:
		ECSOutputArchive(std::string filePath);
		void operator()(entt::entity ent);
		void operator()(std::underlying_type_t<entt::entity> u);
		template <typename T>
		void operator()(entt::entity ent, const T& t);
		void Close();
		std::string AsString();

	private:
		nlohmann::json m_Root;
		nlohmann::json m_Current;

		std::string m_FilePath;
	};

	class ECSInputArchive
	{
	public:
		ECSInputArchive(std::string filePath);
		void operator()(entt::entity& ent);
		void operator()(std::underlying_type_t<entt::entity>& u);
		template <typename T>
		void operator()(entt::entity& ent, T& t);
	private:
		std::string m_FilePath;

		nlohmann::json m_Root;
		nlohmann::json m_Current;

		int m_RootIdx = -1;
		int m_CurrentIdx = 0;
	};


	template <typename Comp, typename... Others>
	std::vector<Entity> ECSManager::GetEntities()
	{
		std::vector<Entity> objects{};
		entt::exclude_t<Undeployed> u{};
		auto view = m_Registry.view<Comp, Others...>(u);
		objects.reserve(m_EntityList.size());

		// Get all Entity owning the entities
		for (entt::entity obj : view)
		{
			for (auto ent : m_EntityList)
			{
				if (ent.second->m_Entity == obj)
				{
					objects.emplace_back(ent.second);
					break;
				}
			}
		}

		return objects;
	}

	template <typename T>
	bool ECSManager::EntityHasComponent(Entity object)
	{
		return m_Registry.any_of<T>(object->m_Entity);
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
			TRE_CORE_ERROR("[" + funcName + "] Object is no longer valid (this or entity is nullptr)");
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
			if (typeid(T) == typeid(Properties))
			{
				TRE_CORE_ERROR("[" + funcName + "] Object does not have the component " + compName);
			}
			else
			{
				TRE_CORE_ERROR("[" + funcName + "] " + GetComponent<Properties>().m_Name + " does not have the component " + compName);
			}
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

	template <typename T>
	void ECSOutputArchive::operator()(entt::entity ent, const T& t)
	{

		if (ECSManager::Instance().GetRegistry().valid(ent))
		{
			m_Current.push_back(static_cast<uint32_t>(ent)); // persist the entity id of the following component

			nlohmann::json json = t;
			m_Current.push_back(json);
		}
	}

	template <typename T>
	void ECSInputArchive::operator()(entt::entity& ent, T& t)
	{
		nlohmann::json componentData = m_Current[m_CurrentIdx * 2];

		auto comp = componentData.get<T>();
		t = comp;

		// Last element is the entID
		uint32_t entID = m_Current[m_CurrentIdx * 2 - 1];
		ent = entt::entity(entID); 
		m_CurrentIdx++;
	}
}