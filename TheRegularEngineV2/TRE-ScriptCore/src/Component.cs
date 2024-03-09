using GlmSharp;

namespace TRE
{
	// Using EntityID instead of EntityID in case need to change again
	using EntityID = System.UInt64;     // Unsigned long long

	// using EntityID = System.UInt64;

	public abstract class Component
	{
		public Entity entity { get; set; }
	}

	public class Transform : Component
	{
		public vec3 Position
		{
			get
			{
				TransformSystem.Engine_GetPosition(entity.ID, out vec3 pos);
				return pos;
			}
			set
			{
				TransformSystem.Engine_SetPosition(entity.ID, value);
			}
		}

		public vec3 Rotation
		{
			get
			{
				TransformSystem.Engine_GetRotation(entity.ID, out vec3 rot);
				return rot;
			}
			set
			{
				TransformSystem.Engine_SetRotation(entity.ID, value);
			}
		}

		public vec3 Scale
		{
			get
			{
				TransformSystem.Engine_GetScaling(entity.ID, out vec3 scale);
				return scale;
			}
			set
			{
				TransformSystem.Engine_SetScaling(entity.ID, value);
			}
		}

		// Constructor for Transform
		public Transform()
		{
			// LEAVE AS BLANK!
		}

		public Transform(Entity e)
		{
			entity = e;
		}

		// Constructor for Transform
		public Transform(vec3 position, vec3 rotation, vec3 scale)
		{
			// Set the default values
			Position = position;
			Rotation = rotation;
			Scale = scale;
		}
	}

	public class Parenting : Component
	{
		public Entity parent
		{
			get
			{
				return ParentingSystem.GetParent(entity.ID);
			}
		}

		public void SetParent(Entity parent)
		{
			ParentingSystem.Engine_ParentSetParent(entity.ID, parent.ID);
		}

		public void RemoveParent()
		{
			ParentingSystem.Engine_ParentRemoveParent(entity.ID);
		}

		public void AddChild(Entity child)
		{
			ParentingSystem.Engine_ParentAddChild(entity.ID, child.ID);
		}

		public void RemoveChild(Entity child)
		{
			ParentingSystem.Engine_ParentRemoveChild(entity.ID, child.ID);
		}

		public Entity GetChild(int _index)
		{
			EntityID childID = ParentingSystem.Engine_GetChildID(entity.ID, _index);
			string childName = ECSManager.Engine_FindNameFromID(childID);
			Entity child = new Entity(childID, childName);
			return child;
		}

		public Entity GetChildFromName(string name)
		{
			EntityID childID = ParentingSystem.Engine_GetChildIDFromName(entity.ID, name);
			string childName = ECSManager.Engine_FindNameFromID(childID);
			Entity child = new Entity(childID, childName);
			return child;
		}

		public int GetTotalChildren()
		{
			return ParentingSystem.Engine_GetTotalChildren(entity.ID);
		}
	}

	public class MeshRenderer : Component
	{
		public string Mesh
		{
			set
			{
				MeshRendererSystem.Engine_SetMesh(entity.ID, value);
			}
			//get
			//{
			//	//return MeshRendererSystem.Engine_GetMesh(entity.ID);
			//}
		}

		public string Material
		{
			set
			{
				MeshRendererSystem.Engine_SetMaterialInstance(entity.ID, value);
			}
			//get
			//{
			//	return MeshRendererSystem.Engine_GetMaterialInstance(entity.ID);
			//}
		}

		public string AnimMaterial
		{
			set
			{
				MeshRendererSystem.Engine_SetAnimMaterialInstance(entity.ID, value);
			}
			//get
			//{
			//	return MeshRendererSystem.Engine_GetAnimMaterialInstance(entity.ID);
			//}
		}

		public bool Visible
		{
			get
			{
				return MeshRendererSystem.Engine_GetMeshVisibility(entity.ID);
			}
			set
			{
				MeshRendererSystem.Engine_SetMeshVisibility(entity.ID, value);
			}
		}
	}

	public class Animation : Component
	{
		public float AnimationSpeed
		{
			get
			{
				return AnimationSystem.Engine_GetAnimationSpeed(entity.ID);
			}
			set
			{
				AnimationSystem.Engine_SetAnimationSpeed(entity.ID, value);
			}
		}
	}

	public class Camera : Component
	{
		// use transform component as template
		// add Camera variables here

		// Can leave as blank for now (ZR)
	}

	public class Rigidbody : Component
	{
		public Rigidbody()
		{
			// LEAVE AS BLANK!
		}
		public bool useGravity
		{
			get
			{
				return RigidBodySystem.Engine_GetGravity(entity.ID);
			}
			set
			{
				RigidBodySystem.Engine_SetGravity(entity.ID, value);
			}
		}
		public bool isKinematic
		{
			get
			{
				return RigidBodySystem.Engine_GetKinematic(entity.ID);
			}
			set
			{
				RigidBodySystem.Engine_SetKinematic(entity.ID, value);
			}
		}
	}

	public class BoxCollider : Component
	{
		public BoxCollider()
		{
			// LEAVE AS BLANK!
		}
		public vec3 HalfExtents
		{
			get
			{
				return PhysicsSystem.Engine_GetBoxColliderHalfExtents(entity.ID);
			}
			set
			{
				PhysicsSystem.ResizeBoxCollider(entity.ID, value);
			}
		}
		public vec3 Offset // SET DOES NOT WORK
		{
			get
			{
				return PhysicsSystem.Engine_GetColliderOffset(entity.ID);
			}
			set
			{
				PhysicsSystem.Engine_UpdateColliderOffset(entity.ID, value);
			}
		}
		public bool IsActive
		{
			get
			{
				return PhysicsSystem.Engine_GetIsActive(entity.ID);
			}
			set
			{
				PhysicsSystem.Engine_SetIsActive(entity.ID, value);
			}
		}
		public bool IsTrigger
		{
			get
			{
				return PhysicsSystem.Engine_GetBoxTrigger(entity.ID);
			}
			set
			{
				PhysicsSystem.Engine_SetBoxTrigger(entity.ID, value);
			}
		}
	}

	public class SphereCollider : Component
	{
		public SphereCollider()
		{
			// LEAVE AS BLANK!
		}
		public float Radius
		{
			get
			{
				return PhysicsSystem.Engine_GetSphereColliderRadius(entity.ID);
			}
			set
			{
				PhysicsSystem.ResizeSphereCollider(entity.ID, value);
			}
		}
		public vec3 Offset // SET DOES NOT WORK
		{
			get
			{
				return PhysicsSystem.Engine_GetColliderOffset(entity.ID);
			}
			set
			{
				PhysicsSystem.Engine_UpdateColliderOffset(entity.ID, value);
			}
		}
		public bool IsActive
		{
			get
			{
				return PhysicsSystem.Engine_GetIsActive(entity.ID);
			}
			set
			{
				PhysicsSystem.Engine_SetIsActive(entity.ID, value);
			}
		}
		public bool IsTrigger
		{
			get
			{
				return PhysicsSystem.Engine_GetSphereTrigger(entity.ID);
			}
			set
			{
				PhysicsSystem.Engine_SetSphereTrigger(entity.ID, value);
			}
		}
	}

	public class CapsuleCollider : Component
	{
		public CapsuleCollider()
		{
			// LEAVE AS BLANK!
		}
		public float Radius
		{
			get
			{
				return PhysicsSystem.Engine_GetCapsuleColliderRadius(entity.ID);
			}
			set
			{
				PhysicsSystem.Engine_ResizeCapsuleCollider(entity.ID, value, PhysicsSystem.Engine_GetCapsuleColliderHalfHeight(entity.ID));
			}
		}
		public float HalfHeight
		{
			get
			{
				return PhysicsSystem.Engine_GetCapsuleColliderHalfHeight(entity.ID);
			}
			set
			{
				PhysicsSystem.Engine_ResizeCapsuleCollider(entity.ID, PhysicsSystem.Engine_GetCapsuleColliderRadius(entity.ID), value);
			}
		}
		public vec3 Offset // SET DOES NOT WORK
		{
			get
			{
				return PhysicsSystem.Engine_GetColliderOffset(entity.ID);
			}
			set
			{
				PhysicsSystem.Engine_UpdateColliderOffset(entity.ID, value);
			}
		}
		public bool IsActive
		{
			get
			{
				return PhysicsSystem.Engine_GetIsActive(entity.ID);
			}
			set
			{
				PhysicsSystem.Engine_SetIsActive(entity.ID, value);
			}
		}
		public bool IsTrigger
		{
			get
			{
				return PhysicsSystem.Engine_GetCapsuleTrigger(entity.ID);
			}
			set
			{
				PhysicsSystem.Engine_SetCapsuleTrigger(entity.ID, value);
			}
		}
	}

	public class CylinderCollider : Component
	{
		public CylinderCollider()
		{
			// LEAVE AS BLANK
		}
		public float Radius
		{
			get
			{
				return PhysicsSystem.Engine_GetCylinderColliderRadius(entity.ID);
			}
			set
			{
				PhysicsSystem.Engine_ResizeCylinderCollider(entity.ID, value, PhysicsSystem.Engine_GetCylinderColliderHeight(entity.ID));
			}
		}
		public float Height
		{
			get
			{
				return PhysicsSystem.Engine_GetCylinderColliderHeight(entity.ID);
			}
			set
			{
				PhysicsSystem.Engine_ResizeCylinderCollider(entity.ID, PhysicsSystem.Engine_GetCylinderColliderRadius(entity.ID), value);
			}
		}
		public vec3 Offset // SET DOES NOT WORK
		{
			get
			{
				return PhysicsSystem.Engine_GetColliderOffset(entity.ID);
			}
			set
			{
				PhysicsSystem.Engine_UpdateColliderOffset(entity.ID, value);
			}
		}
		public bool IsActive
		{
			get
			{
				return PhysicsSystem.Engine_GetIsActive(entity.ID);
			}
			set
			{
				PhysicsSystem.Engine_SetIsActive(entity.ID, value);
			}
		}
		public bool IsTrigger
		{
			get
			{
				return PhysicsSystem.Engine_GetCylinderTrigger(entity.ID);
			}
			set
			{
				PhysicsSystem.Engine_SetCylinderTrigger(entity.ID, value);
			}
		}
	}

	public class Audio : Component
	{
		public Audio()
		{
			// LEAVE AS BLANK!
		}
		public string FileName
		{
			get
			{
				return AudioSystem.Engine_GetFileName(entity.ID);
			}
			set
			{
				AudioSystem.Engine_SetFileName(entity.ID, value);
			}
		}
		public bool IsPlaying()
		{
			return AudioSystem.Engine_GetIsPlaying(entity.ID);
		}
		public void Play()
		{
			AudioSystem.Play(entity.ID);
		}
		public void Pause()
		{
			AudioSystem.Engine_TogglePause(entity.ID);
		}
		public void Stop()
		{
			AudioSystem.Stop(entity.ID);
		}
	}

	public class SpriteRenderer : Component
	{
		public SpriteRenderer()
		{
			// LEAVE AS BLANK!
		}
		public bool isVisible
		{
			get
			{
				return SpriteSystem.Engine_GetVisible(entity.ID);
			}
			set
			{
				SpriteSystem.Engine_SetVisible(entity.ID, value);
			}
		}
		public int Width
		{
			get
			{
				return SpriteSystem.Engine_GetWidth(entity.ID);
			}
			set
			{
				SpriteSystem.Engine_SetWidth(entity.ID, value);
			}
		}
		public int Height
		{
			get
			{
				return SpriteSystem.Engine_GetHeight(entity.ID);
			}
			set
			{
				SpriteSystem.Engine_SetHeight(entity.ID, value);
			}
		}
		public vec4 Color
		{
			get
			{
				return SpriteSystem.Engine_GetColor(entity.ID);
			}
			set
			{
				SpriteSystem.Engine_SetColor(entity.ID, value);
			}
		}
		public string Texture
		{
			get
			{
				return SpriteSystem.Engine_GetTexture(entity.ID);
			}
			set
			{
				SpriteSystem.Engine_SetTexture(entity.ID, value);
			}
		}
	}

	public class UI : Component
	{
		public UI()
		{

		}
		public bool IsVisible
		{
			get
			{
				return UISystem.Engine_GetVisible(entity.ID);
			}
			set
			{
				UISystem.Engine_SetVisible(entity.ID, value);
			}
		}
	}

	public class DirectPathfinding : Component
	{
		public bool isRunning
		{
			get
			{
				return DirectPathfindingSystem.Engine_GetPathfindingRunning(entity.ID);

			}
			set
			{
				if (value)
				{
					DirectPathfindingSystem.Engine_ResumePathfinding(entity.ID);
				}
				else
				{
					DirectPathfindingSystem.Engine_PausePathfinding(entity.ID);
				}
			}
		}

		public vec3 oldPosition
		{
			get
			{
				return DirectPathfindingSystem.Engine_GetOldPositionPathfinding(entity.ID);
			}
		}

		public void Start()
		{
			DirectPathfindingSystem.Engine_StartPathfinding(entity.ID);
		}

		public void Reset()
		{
			DirectPathfindingSystem.Engine_ResetPathfinding(entity.ID);
		}
	}

	public class Text : Component
	{
		public Text()
		{

		}

		public bool IsVisible
		{
			get
			{
				return TextSystem.Engine_GetTextVisible(entity.ID);
			}
			set
			{
				TextSystem.Engine_SetTextVisible(entity.ID, value);
			}
		}

		public string TextMessage
		{
			get
			{
				return TextSystem.Engine_GetTextMessage(entity.ID);
			}
			set
			{
				TextSystem.Engine_SetTextMessage(entity.ID, value);
			}
		}
	}

	public class Particle : Component
	{
		public bool IsActive
		{
			get
			{
				return ParticleSystem.GetActive(entity.ID);
			}
			set
			{
				ParticleSystem.SetActive(entity.ID, value);
			}
		}

	}

}
