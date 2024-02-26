#pragma once

#include "Core/ECS.h"

namespace TRE
{
	class ParentingSystem : public ECSSystem
	{
	public:
		ParentingSystem() = default;
		~ParentingSystem() = default;

		void Update() override;
		void GameUpdate() override;
		void LateUpdate() override;
		void AfterReset() override;
		void OnDestroyEntities() override;
		void Shutdown() override;

		/* !
		@function		SetParent
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			child	Entity to set the parent
		@params			parent	Entity for the new parent

		@brief			Abandons the previous parent if it exist
						Set the parent of the Entity
						Automatically add this Entity to the parent's children list

		Example:
		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);
		*//*__________________________________________________________________________*/
		void SetParent(Entity child, Entity parent);

		/* !
		@function		GetParent
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			child	Entity to get from

		@brief			Returns the parent of the Entity
						Returns nullptr if it doesn't exist

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);

		Entity AccessEntityVarParent = goVar->GetParent();
		*//*__________________________________________________________________________*/
		Entity GetParent(Entity child);

		/* !
		@function		RemoveParent
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			child	Entity to get from

		@brief			Remove the parent of the Entity
						Automatically Abandons this Entity from the parent's children list

		Example:
		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);

		goVar->RemoveParent();
		*//*__________________________________________________________________________*/
		void RemoveParent(Entity child);

		/* !
		@function		AddChild
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			parent	Entity of the parent
		@params			child	Entity for the child

		@brief			Add a child to this Entity children list
						Automatically set this Entity as the parent of the child

		Example:
		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goChildVar = ECSManager::Instance().CreateEntity("goChildVar");

		goVar->AddChild(goChildVar);
		*//*__________________________________________________________________________*/
		void AddChild(Entity parent, Entity child);

		/* !
		@function		GetChildren
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			parent	Entity to get from

		@brief			Returns the vector of children of the Entity

		Example:
		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);

		std::vector<Entity> goParentVarChildren = goParentVar->GetChildren();
		*//*__________________________________________________________________________*/
		std::vector<Entity> GetChildren(Entity parent);

		/* !
		@function		AbandonChild
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			parent	Entity to get from
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
		void AbandonChild(Entity parent, Entity child);

		/* !
		@function		AbandonChildren
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			parent	Entity to get from

		@brief			Remove all children from children list
						Automatically remove parent from each child Entity

		Example:
		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);

		goParentVar->AbandonChildren();
		*//*__________________________________________________________________________*/
		void AbandonChildren(Entity parent);

		void GetTotalEntities(int& noOfEntities, Entity object);

		void UpdateChildTransform(Entity parent, bool updateLocal = false);

		void UpdateChildActive(Entity parent);
	private:
		void UpdateChildLocalData(Entity parent, Entity child);
		void UpdateLocalData(Entity current);
	};
}