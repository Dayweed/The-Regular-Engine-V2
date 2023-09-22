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
#include "Core/Logger.h"
#include "entt.hpp"
#include "ComponentManager.h"
#include "System.h"

#include <typeindex>
#include <nlohmann/json.hpp>

/*                                                                 defines
----------------------------------------------------------------------------- */
#define ECS_ENTITY_DEFAULT_NAME "Default_Entity_Name"
#define ECS_CLONE_DEFAULT_NAME "Clone_Entity_Name"

namespace TRE
{
	class Ent;
	typedef std::shared_ptr<Ent> Entity;

	// NOT the GUID, this is to return the entt::entity id if needed
	typedef std::uint32_t ENTTID;

	// DO NOT USE THIS UNLESS YOU WANT THE Entity TO BE DELETED!
	// Get this component in GetEntities to get Entity that are going to be deleted in this loop
	class Removal
	{
		//static constexpr auto in_place_delete = false;
		bool m_Fake; //This value is to ensure it can compile and be registered
	};
	// DO NOT USE THIS UNLESS IT IS CREATING ALLOCATED ENTITIES!
	// Get this component in GetEntities to get Entity that are allocated and not used in MemoryManager
	class Undeployed
	{
		bool m_Fake; //This value is to ensure it can compile and be registered
	};


	struct NESTCOMP
	{
		char arr_c;

		NESTCOMP() = default;
		~NESTCOMP() = default;

		// Can use NLOHMANN_DEFINE_TYPE_INTRUSIVE even with functions
		void UselessFunction()
		{
			return;
		}

		// Use this if dont have struct/class variables
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(NESTCOMP, arr_c)
	};

	struct FEL : property::base
	{
		std::vector<float> vec_i{};
		float arr_i[3]{};
		NESTCOMP nestedstruct{};
		std::string tobeignored{ "(>-<)" };

		FEL() = default;
		~FEL() = default;
		property_vtable()           // Allows the base class to get these properties  

		// MUST Use BOTH of this if have variables that are struct/class to serialize
		friend void to_json(nlohmann::json& j, const FEL&f) // Serialize
		{
			j = nlohmann::json{
				{ "vector", f.vec_i },
				{ "array", f.arr_i },
				{ "nested", f.nestedstruct }
			};
		}
		friend void from_json(const nlohmann::json& j, FEL& f) // Deserialize
		{
				f.vec_i = j.at("vector").get<std::vector<float>>();
				j.at("array").get_to(f.arr_i);
				j.at("nested").get_to(f.nestedstruct);
		}
	};

	struct FAKEFEL : property::base
	{
		std::string fakeValue{ "NULL" };

		property_vtable()           // Allows the base class to get these properties  

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(FAKEFEL, fakeValue)
	};

	struct Properties : property::base
	{
		std::string m_GUID{};
		bool m_Active{ true };		// To check if it is active
		std::string m_Name{};		// To get the name

		Properties() = default;
		~Properties() = default;

		property_vtable()           // Allows the base class to get these properties  

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Properties, m_Active, m_GUID, m_Name)
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
		@function		GetENTTID
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Returns the Entity ENTTID

		[Warning]		This ENTTID IS NOT the GUID!!!
						Used for getting uint32_t data type for the entt::entity
						that may change upon serializing and deserialzing

						Usage may be for one-time running a container that does not
						need it to be consistent

		Example:
		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		std::cout << goVar->GetENTTID();

		// After serializing and deserializing the scene...

		std::cout << goVar->GetENTTID();

		Output:
		0
		1023479
		*//*__________________________________________________________________________*/
		ENTTID GetENTTID();

		/* !
		@function		GetName
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Returns the Entity Name for less typing

		Example:
		Entity goVar = ECSManager::Instance().CreateEntity("goVarName");
		std::cout << goVar->GetComponent<Properties>().m_Name;
		std::cout << goVar->GetName();

		Output:
		goVarName
		goVarName
		*//*__________________________________________________________________________*/
		std::string GetName();

		/* !
		@function		GetGUID
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Returns the Entity GUID for less typing

		Example:
		Entity goVar = ECSManager::Instance().CreateEntity("goVarName");
		std::cout << goVar->GetComponent<Properties>().m_GUID;
		std::cout << goVar->GetGUID();

		Output:
		"{03A691F2-02DB-4957-8A0E-CA2BDAB9CDF8}"
		"{03A691F2-02DB-4957-8A0E-CA2BDAB9CDF8}"
		*//*__________________________________________________________________________*/
		std::string GetGUID();
		
	private:
		friend class ECSManager;
		friend class MemoryManager;
		friend class PrefabSystem;

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
		@function		DestroyEntity
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			object	Entity to be destroyed

		@brief			Destroys Entity instantly

		Example:
		Entity goVar = ECSManager::Instance().CreateEntity("goVar");

		std::cout << goVar << std::endl; // Address of goVar

		ECSManager::Instance().DestroyEntity(goVar);
		*//*__________________________________________________________________________*/
		void DestroyEntity(Entity& object);

		/* !
		@function		MarkForDeletion
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			object	Entity to be destroyed

		@brief			Marks an Entity for deletion, it would run through normal
						gameloop first before deleting

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


		template <typename T>
		void RegisterComponent(std::string name, bool hidden = false, bool removable = true);

		bool IsRemovableComponent(std::string compName);

		std::vector<std::pair<std::string, property::base*>> GetAllInspectableComponents(Entity object);

		std::vector<std::string> GetAllNonAddedComponents(Entity object);

		void AddCompFromName(Entity ent, std::string compName);

		void RemCompFromName(Entity ent, std::string compName);

		// TODELETE
		void TESTRUN();
		void STRESSTEST();

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
		std::unordered_map<ENTTID, Entity> m_EnttIDList;

		std::unordered_map<entt::id_type, std::string> m_PropertyBased; // ID type, name, 

		std::map<std::string, std::function<void(Entity)>> m_AddCompFunctions{};	// Add
		std::map<std::string, std::function<void(Entity)>> m_RemCompFunctions{};	// Remove
		std::map<std::string, bool> m_CompRemovable{};		// Removable

		template <typename T>
		static void AddEntityComponent(Entity ent)
		{
			ent->AddComponent<T>();
		}

		template <typename T>
		static void RemoveEntityComponent(Entity ent)
		{
			ent->RemoveComponent<T>();
		}

		/*template <typename T>
		static void TESTFUNCTION(int ent)
		{
			std::cout << ">>>> " << typeid(T).name() << "|" << ent << "\n";
		}*/

		template <typename FUNCTION1, typename FUNCTION2>
		void CompFunction(std::string name, FUNCTION1&& func1, FUNCTION2&& func2)
		{
			m_AddCompFunctions.emplace(std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple(std::forward<FUNCTION1>(func1)));
			m_RemCompFunctions.emplace(std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple(std::forward<FUNCTION2>(func2)));
		}
	};

	class ECSOutputArchive
	{
	public:
		ECSOutputArchive(std::string fileName);
		void operator()(entt::entity ent);
		void operator()(std::underlying_type_t<entt::entity> u);
		template <typename T>
		void operator()(entt::entity ent, const T& t);
		void Close();
		std::string AsString();
		std::string GetFilePath();

	private:
		nlohmann::json m_Root;
		nlohmann::json m_Current;

		std::string m_FileName;
	};

	class ECSInputArchive
	{
	public:
		ECSInputArchive(std::string fileName);
		void operator()(entt::entity& ent);
		void operator()(std::underlying_type_t<entt::entity>& u);
		template <typename T>
		void operator()(entt::entity& ent, T& t);
	private:
		std::string m_FileName;

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
			if (m_EnttIDList.find(static_cast<ENTTID>(obj)) != m_EnttIDList.end())
			{
				objects.emplace_back(m_EnttIDList[static_cast<ENTTID>(obj)]);
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
	void ECSManager::RegisterComponent(std::string name, bool hidden, bool removable)
	{
		// m_Components.insert({ hashcode, name });	// This works too
		ComponentManager::Instance().RegisterComponent<T>(name, hidden);

		// Check if this is derived from property::base, used to get all inspectable components
		if (std::is_base_of<property::base, T>::value == true)
		{
			m_PropertyBased.emplace(std::piecewise_construct, std::forward_as_tuple(entt::type_hash<T>::value()), std::forward_as_tuple(name));
		}

		// Ensure entt knows this component exist
		m_Registry.view<T>();

		// Prepare map for ImGui
		CompFunction(name, AddEntityComponent<T>, RemoveEntityComponent<T>);
		m_CompRemovable.emplace(std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple(removable));
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
			TRE_CORE_WARN("[" + funcName + "] Component " + compName + " is already in " + GetComponent<Properties>().m_Name + "...");
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

		if (!componentData.is_null() && (m_CurrentIdx * 2 - 1) < m_Current.size())
		{
			auto comp = componentData.get<T>();
			t = comp;

			// Last element is the entID
			uint32_t entID = m_Current[m_CurrentIdx * 2 - 1];
			ent = entt::entity(entID);
			m_CurrentIdx++;
		}
		else if (componentData.is_null())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			TRE_CORE_ERROR("[" + funcName + "] Component " + compName + " is NULL");
			assert(!componentData.is_null());
		}
		else
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			TRE_CORE_ERROR("[" + funcName + "] Component " + compName + " have an index [" + std::to_string(m_CurrentIdx * 2 - 1) + "] that is >= container size of [" + std::to_string(m_Current.size()) + "]\n");
			assert((m_CurrentIdx * 2 - 1) < m_Current.size());
		}
	}

}

property_begin(TRE::Properties)
{
	property_var(m_Name).Name("Name"),
	property_var(m_Active).Name("Active")
} property_vend_h(TRE::Properties)

property_begin(TRE::FEL)
{
	property_var(vec_i).Name("vec_i"),
	property_var(tobeignored).Name("tobeignored")
} property_vend_h(TRE::FEL)

property_begin(TRE::FAKEFEL)
{
	property_var(fakeValue).Name("fakeValue")
} property_vend_h(TRE::FAKEFEL)
