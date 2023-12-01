using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO.MemoryMappedFiles;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;

using GlmSharp; // Add this for GLM lib
using Coroutine; // Add this for Coroutine lib


/*
 *  This is the core scripting library for TRE.
 *  All bindings 
 *
 *
 *
 */
namespace TRE
{
	// Using EntityID instead of EntityID in case need to change again
	using EntityID = System.UInt64;     // Unsigned long long

	public class Entity : Component
	{
		public EntityID ID;             // Can hold id of entity or id of prefab resource
		public string name;
		public Parenting parenting;
		public Transform transform;

		public Entity()
		{
			ID = new EntityID();
			name = "";
			parenting = null;
			transform = null;
		}

		public Entity(EntityID id)
		{
			ID = id;

			if (ECSManager.Engine_IsValidEntity(ID))
			{
				name = ECSManager.Engine_FindNameFromID(ID);
				parenting = GetComponent<Parenting>();
				transform = GetComponent<Transform>();
			}
			else
			{
				name = "";
				parenting = null;
				transform = null;
			}
		}

		public Entity(EntityID _id, string _name)
		{
			ID = _id;
			name = _name;

			if (ECSManager.Engine_IsValidEntity(ID))
			{
				parenting = GetComponent<Parenting>();
				transform = GetComponent<Transform>();
			}
			else
			{
				parenting = null;
				transform = null;
			}
		}

		public void DestroySelf()
		{
			ECSManager.Engine_DestroyEntity(this.ID);
		}

		public void Rename(string _name)
		{
			name = _name;
			EngineRename(ID, name);
		}

		public void SetActive(bool active)
		{
			EngineSetActive(ID, active);
		}

		public bool GetActive()
		{
			return EngineGetActive(ID);
		}

		public void SetTag(string tag)
		{
			EngineSetTag(ID, tag);
		}

		public string GetTag()
		{
			return EngineGetTag(ID);
		}

		public bool CompareTag(string otherTag)
		{
			return EngineCompareTag(ID, otherTag);
		}

		// Can only be done for Scripting for now

		public bool HasComponent<T>() where T : Component, new()
		{
			return ECSManager.Engine_HasComponent(ID, typeof(T));
		}

		public T GetComponent<T>() where T : Component, new()
		{
			if (Script.Engine_IsScript(typeof(T).ToString())) return Script.Engine_GetScript<T>(ID, typeof(T).ToString());

			return GetCoreComponent<T>();   // To change for getting directly

		}

		private T GetCoreComponent<T>() where T : Component, new()
		{
			if (!HasComponent<T>())
			{
				Debug.Engine_LogError("DO NOT HAVE COMPONENT " + typeof(T).ToString());
				return null;
			}

			T component = new T() { entity = this };
			return component;
		}


		public T AddComponent<T>() where T : Component, new()
		{
			if (ECSManager.Engine_HasComponent(ID, typeof(T)) == true)
			{
				Debug.Engine_LogWarning("Component already exists, returning...");
				return GetCoreComponent<T>();
			}
			ECSManager.Engine_AddComponent(ID, typeof(T));
			return GetCoreComponent<T>();
		}

		public void RemoveComponent<T>() where T : Component, new()
		{
			if (HasComponent<T>())
			{
				ECSManager.Engine_RemoveComponent(ID, typeof(T));
			}
		}

		public static void EngineRename(EntityID id, string name)
		{
			Engine_Rename(id, name);
		}

		public static void EngineSetActive(EntityID id, bool active)
		{
			Engine_SetActive(id, active);
		}

		public static bool EngineGetActive(EntityID id)
		{
			return Engine_GetActive(id);
		}

		public static void EngineSetTag(EntityID id, string tag)
		{
			Engine_SetTag(id, tag);
		}

		public static string EngineGetTag(EntityID id)
		{
			return Engine_GetTag(id);
		}

		public static bool EngineCompareTag(EntityID id, string otherTag)
		{
			return Engine_CompareTag(id, otherTag);
		}



		// DONT USE THIS, INCOMPLETE AND UNTESTED
		/*
		public T AddComponent<T>() where T : new()
		{
			if (Script.IsScript(typeof(T).ToString())) return Script.GetScript<T>(ID, typeof(T).ToString());

			return GenerateComponent<T>();
		}
		*/

		// DONT USE THIS, INCOMPLETE AND UNTESTED
		/*
		private T GenerateComponent<T>() where T : new()
		{
			if (typeof(T).ToString() == typeof(Parenting).ToString()) return (T) Convert.ChangeType(parenting, typeof(T));
			if (typeof(T).ToString() == typeof(Transform).ToString()) return (T) Convert.ChangeType(transform, typeof(T));

			return new T();
		}
		*/

		// Private binded calls
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_Rename(EntityID id, string name);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetActive(EntityID id, bool active);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Engine_GetActive(EntityID id);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetTag(EntityID id, string tag);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Engine_GetTag(EntityID id);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Engine_CompareTag(EntityID id, string otherTag);
	}

	public struct Prefab
	{
		public bool IsPrefabResource(EntityID id)
		{
			return Engine_IsPrefabResource(id);
		}

		public EntityID CreatePrefabEntity(EntityID prefabid)
		{
			return Engine_CreatePrefabEntity(prefabid);
		}

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Engine_IsPrefabResource(EntityID id);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static EntityID Engine_CreatePrefabEntity(EntityID prefabid/*, Vector3 postion = new Vector3(), Vector3 rotation = new Vector3(), Vector3 scaling = new Vector3()*/);
	}

	public class ParentingSystem
	{
		public static Entity GetParent(EntityID id)
		{
			EntityID parentID = ECSManager.Engine_FindParentIDFromID(id);
			string parentName = ECSManager.Engine_FindNameFromID(parentID);
			Entity parent = new Entity(parentID, parentName);
			return parent;
		}

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static EntityID Engine_GetChildID(EntityID _id, int _index);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static EntityID Engine_GetChildIDFromName(EntityID _id, string name);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_ParentSetParent(EntityID _id, EntityID _parent_id);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_ParentRemoveParent(EntityID _id);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_ParentAddChild(EntityID _id, EntityID _child_id);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_ParentRemoveChild(EntityID _id, EntityID _child_id);
	}

	public class Debug
	{
		public static void Log(string message)
		{
			Engine_Log(message);
		}
		public static void LogWarning(string message)
		{
			Engine_LogWarning(message);
		}
		public static void LogError(string message)
		{
			Engine_LogError(message);
		}
		public static void LogCritical(string message)
		{
			Engine_LogCritical(message);
		}


		// Logging Functions
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_Log(string message);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_LogWarning(string message);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_LogError(string message);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_LogCritical(string message);

		// Debugging Functions

		// ECS get parent

	}

	public class TransformSystem
	{
		public static void SetPosition(EntityID id, vec3 position)
		{
			Engine_SetPosition(id, position);
		}

		public static void GetPosition(EntityID id, out vec3 output)
		{
			Engine_GetPosition(id, out output);
		}

		public static void SetRotation(EntityID id, vec3 rotation)
		{
			Engine_SetRotation(id, rotation);
		}

		public static void GetRotation(EntityID id, out vec3 rotation)
		{
			Engine_GetRotation(id, out rotation);
		}

		public static void SetScaling(EntityID id, vec3 scaling)
		{
			Engine_SetScaling(id, scaling);
		}

		public static void GetScaling(EntityID id, out vec3 output)
		{
			Engine_GetScaling(id, out output);
		}

		public static vec3 RotateVector(vec3 vector, vec3 rotation)
		{
			return Engine_RotateVector(vector, rotation);
		}

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static vec3 Engine_RotateVector(vec3 vector, vec3 rotation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetPosition(EntityID id, out vec3 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetPosition(EntityID id, vec3 position);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetRotation(EntityID id, out vec3 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetRotation(EntityID id, vec3 rotation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetScaling(EntityID id, out vec3 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetScaling(EntityID id, vec3 rotation);
	}

	public class ECSManager
	{
		public static Entity FindEntityByName(string name)
		{
			Entity ent = new Entity(Engine_FindIDFromName(name));
			return ent;
		}
		public static Entity Instantiate(Entity entity/*, Vector3 postion = new Vector3(), Vector3 scaling = new Vector3(), Vector3 rotation = new Vector3()*/)
		{
			// Ensure scaling is not zero, since Vector3 does not allow const version currently and must compile-time const
			//if (scaling.x == 0 || scaling.y == 0 || scaling.z == 0)
			//{
			//	Debug.LogWarning("Scaling is zero, setting the values to 1...");
			//	scaling = new Vector3(1, 1, 1);
			//}

			bool isPrefab = Prefab.Engine_IsPrefabResource(entity.ID);
			if (isPrefab)
			{
				EntityID id = Prefab.Engine_CreatePrefabEntity(entity.ID/*, postion, rotation, scaling*/);
				return new Entity(id, Engine_FindNameFromID(id));
			}
			else if (Engine_IsValidEntity(entity.ID))
			{
				EntityID id = Engine_CloneEntity(entity.ID/*, postion, rotation, scaling*/);
				return new Entity(id, Engine_FindNameFromID(id));
			}
			else
			{
				// Invalid Entity!
				return new Entity(new EntityID(), "");
			}
		}

		public static EntityID CreateEntity(string name, vec3 postion = new vec3(), vec3 rotation = new vec3(),
			vec3 scaling = new vec3())
		{
			return Engine_CreateEntity(name, postion, rotation, scaling);
		}

		public static EntityID CloneEntity(EntityID id)
		{
			return Engine_CloneEntity(id);
		}

		public static bool IsValidEntity(EntityID prefabid)
		{
			return Engine_IsValidEntity(prefabid);
		}

		public static void AddComponent(EntityID entityID, Type component)
		{
			Engine_AddComponent(entityID, component);
		}

		public static void RemoveComponent(EntityID entityID, Type component)
		{
			Engine_RemoveComponent(entityID, component);
		}

		public static void DestroyEntity(EntityID entityID)
		{
			Engine_DestroyEntity(entityID);
		}

		public static EntityID FindIDFromName(string name)
		{
			return Engine_FindIDFromName(name);
		}

		public static string FindNameFromID(EntityID id)
		{
			return Engine_FindNameFromID(id);
		}

		public static EntityID FindParentIDFromID(EntityID id)
		{
			return Engine_FindParentIDFromID(id);
		}

		public static bool HasComponent(EntityID id, Type component)
		{
			return Engine_HasComponent(id, component);
		}

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static EntityID Engine_CreateEntity(string name, vec3 postion = new vec3(), vec3 rotation = new vec3(), vec3 scaling = new vec3());

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static EntityID Engine_CloneEntity(EntityID id/*, Vector3 postion = new Vector3(), Vector3 rotation = new Vector3(), Vector3 scaling = new Vector3()*/);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Engine_IsValidEntity(EntityID prefabid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_AddComponent(EntityID entityID, Type component);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_RemoveComponent(EntityID entityID, Type component);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_DestroyEntity(EntityID entityID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static EntityID Engine_FindIDFromName(string name);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Engine_FindNameFromID(EntityID id);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static EntityID Engine_FindParentIDFromID(EntityID id);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Engine_HasComponent(EntityID id, Type component);
	}

	public class CameraSystem
	{
		public static void SetPosition(EntityID entity, vec3 newPos)
		{
			Engine_SetPosition(entity, newPos);
		}

		public static void SetRotation(EntityID entity, vec3 newRot)
		{
			Engine_SetRotation(entity, newRot);
		}

		public static void SetViewportSize(EntityID entity, vec2 newSize)
		{
			Engine_SetViewportSize(entity, newSize);
		}

		public static void SetFocalPointA(EntityID entity, vec3 newFocalPoint)
		{
			Engine_SetFocalPointA(entity, newFocalPoint);
		}

		public static void SetFocalLength(EntityID entity, float newFocalLength)
		{
			Engine_SetFocalLength(entity, newFocalLength);
		}

		public static void SetPitch(EntityID entity, float newPitch)
		{
			Engine_SetPitch(entity, newPitch);
		}

		public static void SetYaw(EntityID entity, float newYaw)
		{
			Engine_SetYaw(entity, newYaw);
		}

		public static void SetRoll(EntityID entity, float newRoll)
		{
			Engine_SetRoll(entity, newRoll);
		}

		public static void SetFOV(EntityID entity, float newFOV)
		{
			Engine_SetFOV(entity, newFOV);
		}

		public static void SetNear(EntityID entity, float newNear)
		{
			Engine_SetNear(entity, newNear);
		}

		public static void SetFar(EntityID entity, float newFar)
		{
			Engine_SetFar(entity, newFar);
		}

		public static void SetLeft(EntityID entity, float newLeft)
		{
			Engine_SetLeft(entity, newLeft);
		}

		public static void SetRight(EntityID entity, float newRight)
		{
			Engine_SetRight(entity, newRight);
		}

		public static void SetTop(EntityID entity, float newTop)
		{
			Engine_SetTop(entity, newTop);
		}

		public static void SetBottom(EntityID entity, float newBottom)
		{
			Engine_SetBottom(entity, newBottom);
		}

		public static void SetAspectRatio(EntityID entity, float newAspectRatio)
		{
			Engine_SetAspectRatio(entity, newAspectRatio);
		}
		public static void SetIsPerspective(EntityID entity, bool newIsPerspective)
		{
			Engine_SetIsPerspective(entity, newIsPerspective);
		}

		public static void SetIsMainCamera(EntityID entity, bool newIsMainCamera)
		{
			Engine_SetIsMainCamera(entity, newIsMainCamera);
		}

		public static void GetPosition(EntityID entity, out vec3 output)
		{
			Engine_GetPosition(entity, out output);
		}

		public static void GetRotation(EntityID entity, out vec3 output)
		{
			Engine_GetRotation(entity, out output);
		}

		public static void GetViewMatrix(EntityID entity, out mat4 output)
		{
			Engine_GetViewMatrix(entity, out output);
		}

		public static void GetProjectionMatrix(EntityID entity, out mat4 output)
		{
			Engine_GetProjectionMatrix(entity, out output);
		}

		public static void GetInverseViewMatrix(EntityID entity, out mat4 output)
		{
			Engine_GetInverseViewMatrix(entity, out output);
		}

		public static void GetInverseProjectionMatrix(EntityID entity, out mat4 output)
		{
			Engine_GetInverseProjectionMatrix(entity, out output);
		}

		public static void GetInverseViewProjectionMatrix(EntityID entity, out mat4 output)
		{
			Engine_GetInverseViewProjectionMatrix(entity, out output);
		}

		public static void GetViewportSize(EntityID entity, out vec2 output)
		{
			Engine_GetViewportSize(entity, out output);
		}

		public static void GetFocalPoint(EntityID entity, out vec3 output)
		{
			Engine_GetFocalPoint(entity, out output);
		}

		public static void GetFocalLength(EntityID entity, out float output)
		{
			Engine_GetFocalLength(entity, out output);
		}

		public static void GetPitch(EntityID entity, out float output)
		{
			Engine_GetPitch(entity, out output);
		}

		public static void GetYaw(EntityID entity, out float output)
		{
			Engine_GetYaw(entity, out output);
		}

		public static void GetRoll(EntityID entity, out float output)
		{
			Engine_GetRoll(entity, out output);
		}

		public static void GetFOV(EntityID entity, out float output)
		{
			Engine_GetFOV(entity, out output);
		}

		public static void GetNear(EntityID entity, out float output)
		{
			Engine_GetNear(entity, out output);
		}

		public static void GetFar(EntityID entity, out float output)
		{
			Engine_GetFar(entity, out output);
		}

		public static void GetLeft(EntityID entity, out float output)
		{
			Engine_GetLeft(entity, out output);
		}

		public static void GetRight(EntityID entity, out float output)
		{
			Engine_GetRight(entity, out output);
		}

		public static void GetTop(EntityID entity, out float output)
		{
			Engine_GetTop(entity, out output);
		}

		public static void GetBottom(EntityID entity, out float output)
		{
			Engine_GetBottom(entity, out output);
		}

		public static void GetAspectRatio(EntityID entity, out float output)
		{
			Engine_GetAspectRatio(entity, out output);
		}

		public static void GetIsPerspective(EntityID entity, out bool output)
		{
			Engine_GetIsPerspective(entity, out output);
		}

		public static void GetIsMainCamera(EntityID entity, out bool output)
		{
			Engine_GetIsMainCamera(entity, out output);
		}

		public static void SetMainCameraLookAt(vec3 target, float distance)
		{
			Engine_SetMainCameraLookAt(target, distance);
		}

		public static void TransitionMainCamera(vec3 targetPosition, vec3 targetRotation, float speed)
		{
			Engine_TransitionMainCamera(targetPosition, targetRotation, speed);
		}

		public static vec3 GetMainCameraForwardVec()
		{
			return Engine_GetMainCameraForwardVec();
		}

		public static vec3 GetMainCameraRightVec()
		{
			return Engine_GetMainCameraRightVec();
		}

		public static vec3 GetMainCameraRotation()
		{
			return Engine_GetMainCameraRotation();
		}

		#region Setters
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetPosition(EntityID entityid, vec3 newPos);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetRotation(EntityID entityid, vec3 newRot);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetViewportSize(EntityID entityid, vec2 newSize);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetFocalPointA(EntityID entityid, vec3 newFocalPoint);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetFocalLength(EntityID entityid, float newFocalLength);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetPitch(EntityID entityid, float newPitch);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetYaw(EntityID entityid, float newYaw);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetRoll(EntityID entityid, float newRoll);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetFOV(EntityID entityid, float newFOV);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetNear(EntityID entityid, float newNear);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetFar(EntityID entityid, float newFar);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetLeft(EntityID entityid, float newLeft);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetRight(EntityID entityid, float newRight);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetTop(EntityID entityid, float newTop);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetBottom(EntityID entityid, float newBottom);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetAspectRatio(EntityID entityid, float newAspectRatio);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetIsPerspective(EntityID entityid, bool newIsPerspective);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetIsMainCamera(EntityID entityid, bool newIsMainCamera);

		#endregion

		#region Getters
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetPosition(EntityID entityid, out vec3 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetRotation(EntityID entityid, out vec3 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetViewMatrix(EntityID entityid, out mat4 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetProjectionMatrix(EntityID entityid, out mat4 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetInverseViewMatrix(EntityID entityid, out mat4 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetInverseProjectionMatrix(EntityID entityid, out mat4 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetInverseViewProjectionMatrix(EntityID entityid, out mat4 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetViewportSize(EntityID entityid, out vec2 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetFocalPoint(EntityID entityid, out vec3 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetFocalLength(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetPitch(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetYaw(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetRoll(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetFOV(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetNear(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetFar(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetLeft(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetRight(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetTop(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetBottom(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetAspectRatio(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetIsPerspective(EntityID entityid, out bool output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetIsMainCamera(EntityID entityid, out bool output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetMainCameraLookAt(vec3 target, float distance);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_TransitionMainCamera(vec3 targetPosition, vec3 targetRotation, float speed);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static vec3 Engine_GetMainCameraForwardVec();

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static vec3 Engine_GetMainCameraRightVec();

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static vec3 Engine_GetMainCameraRotation();

		#endregion

	}

	public class MeshRendererSystem
	{
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetMaterialInstance(EntityID entityid, string instanceGUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Engine_GetMaterialInstance(EntityID entityid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetAnimMaterialInstance(EntityID entityid, string instanceGUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Engine_GetAnimMaterialInstance(EntityID entityid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetMesh(EntityID entityid, string meshGUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetMeshName(EntityID entityid, string meshName);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Engine_GetMesh(EntityID entityid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetMeshVisibility(EntityID entityid, bool visibility);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Engine_GetMeshVisibility(EntityID entityid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Engine_IsCurrentMesh(EntityID entityid, string meshName);
		public static bool IsCurrentMesh(EntityID entityID, string meshName)
		{
			return Engine_IsCurrentMesh(entityID, meshName);
		}
	}

	public class AnimationSystem
	{
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetAnimationSpeed(EntityID entityid, float aniamtionSpeed);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float Engine_GetAnimationSpeed(EntityID entityid);
	}


	public enum ForceMode
	{
		Force,              //!< parameter has unit of mass * length / time^2, i.e., a force
		Impulse,            //!< parameter has unit of mass * length / time, i.e., force * time
		VelocityChange,     //!< parameter has unit of length / time, i.e., the effect is mass independent: a velocity change.
		Acceleration        //!< parameter has unit of length/ time^2, i.e., an acceleration. It gets treated just like a force except the mass is not divided out before integration.
	}

	public class PhysicsSystem
	{
		public static void ResizeSphereCollider(EntityID entityid, float newRadius)
		{
			Engine_ResizeSphereCollider(entityid, newRadius);
		}
		public static void ResizeBoxCollider(EntityID entityid, vec3 newHalfExtents)
		{
			Engine_ResizeBoxCollider(entityid, newHalfExtents);
		}
		public static void ResizeCapsuleCollider(EntityID entityid, float newRadius, float newHelfHeight)
		{
			Engine_ResizeCapsuleCollider(entityid, newRadius, newHelfHeight);
		}

		public static void UpdateColliderOffset(EntityID entityid, vec3 offset)
		{
			Engine_UpdateColliderOffset(entityid, offset);
		}

		public static void GetColliderOffset(EntityID entityid, out vec3 offset)
		{
			offset = Engine_GetColliderOffset(entityid);
		}

		public static void AddForce(EntityID id, vec3 force, ForceMode mode)
		{
			Engine_AddForce(id, force, mode);
		}

		public static void ConstrainRotationX(EntityID entityid, bool state)
		{
			Engine_ConstrainRotationX(entityid, state);
		}

		public static void ConstrainRotationY(EntityID entityid, bool state)
		{
			Engine_ConstrainRotationY(entityid, state);
		}

		public static void ConstrainRotationZ(EntityID entityid, bool state)
		{
			Engine_ConstrainRotationZ(entityid, state);
		}

		public static void GetLinearVelocity(EntityID entityid, out vec3 output)
		{
			Engine_GetLinearVelocity(entityid, out output);
		}

		public static void SetLinearVelocity(EntityID entityid, vec3 velocity)
		{
			Engine_SetLinearVelocity(entityid, velocity);
		}

		public static bool IsCollisionEnter(EntityID entityid1, EntityID entityid2)
		{
			return Engine_IsCollisionEnter(entityid1, entityid2);
		}

		public static bool IsCollisionStay(EntityID entityid1, EntityID entityid2)
		{
			return Engine_IsCollisionStay(entityid1, entityid2);
		}

		public static bool IsCollisionExit(EntityID entityid1, EntityID entityid2)
		{
			return Engine_IsCollisionExit(entityid1, entityid2);
		}

		public static bool IsTriggerEnter(EntityID entityid1, EntityID entityid2)
		{
			return Engine_IsTriggerEnter(entityid1, entityid2);
		}

		public static bool IsTriggerStay(EntityID entityid1, EntityID entityid2)
		{
			return Engine_IsTriggerStay(entityid1, entityid2);
		}

		public static bool IsTriggerExit(EntityID entityid1, EntityID entityid2)
		{
			return Engine_IsTriggerExit(entityid1, entityid2);
		}



		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_ResizeSphereCollider(EntityID entityid, float newRadius);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float Engine_GetSphereColliderRadius(EntityID entityid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_ResizeBoxCollider(EntityID entityid, vec3 newHalfExtents);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static vec3 Engine_GetBoxColliderHalfExtents(EntityID entityid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_ResizeCapsuleCollider(EntityID entityid, float newRadius, float newHelfHeight);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float Engine_GetCapsuleColliderRadius(EntityID entityid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float Engine_GetCapsuleColliderHalfHeight(EntityID entityid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_UpdateColliderOffset(EntityID entityid, vec3 offset);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static vec3 Engine_GetColliderOffset(EntityID entityid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_AddForce(EntityID entityid, vec3 force, ForceMode mode);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_ConstrainRotationX(EntityID entityid, bool state);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_ConstrainRotationY(EntityID entityid, bool state);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_ConstrainRotationZ(EntityID entityid, bool state);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_GetLinearVelocity(EntityID entityid, out vec3 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetLinearVelocity(EntityID entityid, vec3 velocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Engine_IsCollisionEnter(EntityID entityid1, EntityID entityid2);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Engine_IsCollisionStay(EntityID entityid1, EntityID entityid2);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Engine_IsCollisionExit(EntityID entityid1, EntityID entityid2);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Engine_IsTriggerEnter(EntityID entityid1, EntityID entityid2);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Engine_IsTriggerStay(EntityID entityid1, EntityID entityid2);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Engine_IsTriggerExit(EntityID entityid1, EntityID entityid2);
	}

	public class RigidBodySystem
	{


		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetKinematic(EntityID entityid, bool enable);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Engine_GetKinematic(EntityID entityid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_SetGravity(EntityID entityid, bool enable);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Engine_GetGravity(EntityID entityid);
	}

	public class InputSystem
	{
		public static bool GetKeyDown(InputKeys keycode)
		{
			return Engine_GetKeyDown(keycode);
		}

		public static bool GetKeyTrigger(InputKeys keycode)
		{
			return Engine_GetKeyTrigger(keycode);
		}

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Engine_GetKeyDown(InputKeys keycode);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Engine_GetKeyTrigger(InputKeys keycode);
	}

	public class MathF
	{
		public static float Sqrt(float value)
		{
			return Engine_Sqrt(value);
		}

		public static float Lerp(float start, float end, float t)
		{
			return Engine_Lerp(start, end, t);
		}

		public static vec3 Vec3Lerp(vec3 start, vec3 end, float t)
		{
			return Engine_Vec3Lerp(start, end, t);
		}

		public static float Sin(float value)
		{
			// return Engine_Sin(value);
			return (float)System.Math.Sin(value);
		}

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float Engine_Sqrt(float value);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float Engine_Lerp(float start, float end, float t);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static vec3 Engine_Vec3Lerp(vec3 start, vec3 end, float t);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float Engine_Sin(float value);

	}

	public class Random
	{
		public static int Range(int min_inclusive, int max_exclusive)
		{
			return Engine_IntRange(min_inclusive, max_exclusive);
		}

		public static float Range(float min_inclusive, float max_inclusive)
		{
			return Engine_FloatRange(min_inclusive, max_inclusive);
		}

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static int Engine_IntRange(int min_inclusive, int max_exclusive);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static float Engine_FloatRange(float min_inclusive, float max_inclusive);
	}

	public class Time
	{
		public static float deltaTime
		{
			get
			{
				float time = Engine_GetDeltaTime();
				return time;
			}
			set
			{

			}
		}

		public static float GetDeltaTime()
		{
			return Engine_GetDeltaTime();
		}

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static float Engine_GetDeltaTime();
	}

	public class AudioSystem
	{
		public static void Play(EntityID entityid)
		{
			Engine_Play(entityid);
		}

		public static void PlayOnce(EntityID entityid)
		{
			Engine_PlayOnce(entityid);
		}

		public static void TogglePause(EntityID entityid)
		{
			Engine_TogglePause(entityid);
		}

		public static void Stop(EntityID entityid)
		{
			Engine_Stop(entityid);
		}

		public static bool GetIsPlaying(EntityID entityid)
		{
			return Engine_GetIsPlaying(entityid);
		}


		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static void Engine_Play(EntityID entityid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static void Engine_PlayOnce(EntityID entityid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static void Engine_TogglePause(EntityID entityid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static void Engine_Stop(EntityID entityid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static void Engine_SetFileName(EntityID entityid, string fileName);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static string Engine_GetFileName(EntityID entityid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static bool Engine_GetIsPlaying(EntityID entityid);
	}

	public class Scene
	{
		public static void ChangeScene(string sceneName)
		{
			Engine_ChangeScene(sceneName);
		}

		public static void TransitionScene(string sceneName, float totalDuration)
		{
			Engine_TransitionScene(sceneName, totalDuration);
		}

		public static string GetSceneName()
		{
			return Engine_GetSceneName();
		}

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_ChangeScene(string sceneName);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Engine_TransitionScene(string sceneName, float totalDuration);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Engine_GetSceneName();
	}

	public class Script
	{
		public static bool IsScript(string ClassName)
		{
			return Engine_IsScript(ClassName);
		}

		public static bool HaveScript(EntityID ID, string ClassName)
		{
			return Engine_HaveScript(ID, ClassName);
		}

		public static T GetScript<T>(EntityID ID, string ClassName)
		{
			return Engine_GetScript<T>(ID, ClassName);
		}


		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static bool Engine_IsScript(string ClassName);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static bool Engine_HaveScript(EntityID ID, string ClassName);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static T Engine_GetScript<T>(EntityID ID, string ClassName);
	}

	public class SpriteSystem
	{
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static void Engine_SetVisible(EntityID ID, bool isVisible);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static bool Engine_GetVisible(EntityID ID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static void Engine_SetWidth(EntityID ID, int width);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static int Engine_GetWidth(EntityID ID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static void Engine_SetHeight(EntityID ID, int height);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static int Engine_GetHeight(EntityID ID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static void Engine_SetColor(EntityID ID, vec4 color);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static vec4 Engine_GetColor(EntityID ID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static void Engine_SetTexture(EntityID ID, string texture);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static string Engine_GetTexture(EntityID ID);
	}

	public class Game
	{
		public static void CloseGame()
		{
			Engine_CloseGame();
		}

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static void Engine_CloseGame();
	}

	public class PersistentSystem
	{
		public static string GetValue(string Name)
		{
			return Engine_GetPersistentValue(Name);
		}

		public static void SetValue(string Name, string Value)
		{
			Engine_SetPersistentValue(Name, Value);
		}

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static string Engine_GetPersistentValue(string VariableName);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static void Engine_SetPersistentValue(string VariableName, string VariableValue);
	}

    public class UISystem
    {

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static void Engine_SetVisible(EntityID ID, bool isVisible);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static bool Engine_GetVisible(EntityID ID);
    }

}
