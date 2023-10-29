using System;
using System.Collections.Generic;
using System.IO.MemoryMappedFiles;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;


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
	using EntityID = System.UInt64;		// Unsigned long long

    public class Entity
	{
		public EntityID ID;				// Can hold id of entity or id of prefab resource
		public string name;
		public Parenting parenting;
		public Transform transform;

        protected Entity()
        {
			ID = new EntityID();
			name = "";
            parenting = new Parenting();
			transform = new Transform();
        }

        public Entity(EntityID id)
        {
			ID = id;
			name = ECSManager.FindNameFromID(ID);
        }

        public Entity(EntityID _id = new EntityID(), string _name = "")
		{
			ID = _id;
            name = _name;

            parenting = new Parenting(ID);
			TransformSystem.GetPosition(ID, out Vector3 pos);
			TransformSystem.GetRotation(ID, out Vector3 rot);
            transform = new Transform(ID, pos, rot, new Vector3(1,1,1));
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

        // Private binded calls
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void EngineRename(EntityID id, string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void EngineSetActive(EntityID id, bool active);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool EngineGetActive(EntityID id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void EngineSetTag(EntityID id, string tag);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string EngineGetTag(EntityID id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool EngineCompareTag(EntityID id, string otherTag);
    }

	public struct Prefab
	{
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool EngineIsPrefabResource(EntityID id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static EntityID CreatePrefabEntity(EntityID prefabid, Vector3 postion = new Vector3(), Vector3 rotation = new Vector3(), Vector3 scaling = new Vector3());
    }

	public struct Parenting
	{
        private EntityID id;

        public Parenting(EntityID myID)
		{
			id = myID;
        }

        public void SetParent(Entity _parent)
        {
			EngineParentSetParent(id, _parent.ID);
        }

        public Entity GetParent()
        {
            EntityID parentID = ECSManager.FindParentIDFromID(id);
            string parentName = ECSManager.FindNameFromID(parentID);
            Entity parent = new Entity(parentID, parentName);
            return parent;
        }

        public void RemoveParent()
        {
            EngineParentRemoveParent(id);
        }

		public void AddChild(Entity child)
		{
			EngineParentAddChild(id, child.ID);
		}

		public void RemoveChild(Entity child)
		{
			EngineParentRemoveChild(id, child.ID);
		}

        public Entity GetChild(int _index)
		{
            EntityID childID = EngineGetChildID(id, _index);
			string childName = ECSManager.FindNameFromID(childID);
            Entity child = new Entity(childID, childName);
			return child;
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static EntityID EngineGetChildID(EntityID _id, int _index);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void EngineParentSetParent(EntityID _id, EntityID _parent_id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void EngineParentRemoveParent(EntityID _id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void EngineParentAddChild(EntityID _id, EntityID _child_id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void EngineParentRemoveChild(EntityID _id, EntityID _child_id);
    }

	// Reference to this for what components that can be added to the entity.
	public enum Components
	{
		Mesh = 0,
		Camera = 1,
		Audio = 2,
	}

	#region Vector3
	public struct Vector3
	{
		public float x, y, z;
        public static Vector3 left = new Vector3(-1, 0, 0);
        public static Vector3 right = new Vector3(1, 0, 0);
        public static Vector3 down = new Vector3(0, -1, 0);
        public static Vector3 up = new Vector3(0, 1, 0);
        public static Vector3 back = new Vector3(0, 0, -1);
        public static Vector3 forward = new Vector3(0, 0, 1);
        public static Vector3 zero = new Vector3(0, 0, 0);
        public static Vector3 one = new Vector3(1, 1, 1);

        public Vector3(float x, float y, float z)
		{
			this.x = x; this.y = y; this.z = z;
		}

		public static Vector3 operator +(Vector3 a, Vector3 b)
		{
			return new Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
		}

		public static Vector3 operator +(Vector3 a, float b)
		{
			return new Vector3(a.x + b, a.y + b, a.z + b);
		}

		public static Vector3 operator -(Vector3 a, Vector3 b)
		{
			return new Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
		}

		public static Vector3 operator -(Vector3 a, float b)
		{
			return new Vector3(a.x - b, a.y - b, a.z - b);
		}

		public static Vector3 operator *(Vector3 a, Vector3 b)
		{
			return new Vector3(a.x * b.x, a.y * b.y, a.z * b.z);
		}

		public static Vector3 operator *(Vector3 a, float b)
		{
			return new Vector3(a.x * b, a.y * b, a.z * b);
		}

		public static Vector3 operator /(Vector3 a, Vector3 b)
		{
			return new Vector3(a.x / b.x, a.y / b.y, a.z / b.z);
		}

		public static Vector3 operator /(Vector3 a, float b)
		{
			return new Vector3(a.x / b, a.y / b, a.z / b);
		}

		public static bool operator ==(Vector3 a, Vector3 b)
		{
			if (a.x == b.x && a.y == b.y && a.z == b.z)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		public float Magnitude()
		{
			return (float)Math.Sqrt(x * x + y * y + z * z);
		}

		public void Normalize()
		{
			if(Magnitude() != 0)
			{
                this.x = this.x / Magnitude();
                this.y = this.y / Magnitude();
                this.z = this.z / Magnitude();
            }
			if(Magnitude() == 0)
			{
                this.x = 0;
                this.y = 0;
                this.z = 0;
            }
		}

		public static bool operator !=(Vector3 a, Vector3 b)
		{
			if (a.x != b.x || a.y != b.y || a.z != b.z)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		public static float Distance(Vector3 vec1, Vector3 vec2)
		{
			// TO DO CALCULATE VECTOR DISTANCE :p
			return vec1.Magnitude() - vec2.Magnitude();
		}
	}
	#endregion

	public struct Vector2
	{
		public float x, y;

		public Vector2(float x, float y)
		{
			this.x = x; this.y = y;
		}
	}

	// Use a 2D array for the matrix.
	public struct Mat4x4
	{
		public float[,] matrix;

		public Mat4x4(float[,] matrix)
		{
			this.matrix = matrix;
		}
	}

	public class Core
	{
		// Logging Functions
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Log(string message);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void LogWarning(string message);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void LogError(string message);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void LogCritical(string message);

		// Debugging Functions

		// ECS get parent

	}

	public struct Transform
	{
		private EntityID id;
		public Vector3 position, rotation, scale;

		public Transform(EntityID _id = new EntityID(), Vector3 _pos = new Vector3(), Vector3 _rot = new Vector3(), Vector3 _sca = new Vector3())
		{
			id = _id;
			position = _pos;
			rotation = _rot;
			scale = _sca;
		}

		public void SetPosition(Vector3 output)
		{
			position = output;

            TransformSystem.SetPosition(id, output);
		}

		public void SetRotation(Vector3 output)
		{
			rotation = output;

			TransformSystem.SetRotation(id, output);
		}
	}

	public class TransformSystem
	{
		public void transformDemo(EntityID id)
		{
			Vector3 test = new Vector3(0, 0, 0);
			GetRotation(id, out test);

			test.x += 1;

			SetRotation(id, test);
		}

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetPosition(EntityID id, out Vector3 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetPosition(EntityID id, Vector3 position);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetRotation(EntityID id, out Vector3 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetRotation(EntityID id, Vector3 rotation);
	}

	public class ECSManager
	{
		public static Entity FindEntityByName(string name)
		{
			Entity ent = new Entity();
			ent.ID = FindIDFromName(name);
			ent.name = name;
            TransformSystem.GetPosition(ent.ID, out Vector3 pos);
            TransformSystem.GetRotation(ent.ID, out Vector3 rot);
            ent.transform = new Transform(ent.ID, pos, rot, new Vector3(1, 1, 1));
			return ent;
        }
		public static Entity Instantiate(Entity entity, Vector3 postion = new Vector3(), Vector3 rotation = new Vector3(), Vector3 scaling = new Vector3())
		{
			// Ensure scaling is not zero, since Vector3 does not allow const version currently and must compile-time const
			if (scaling.x == 0 || scaling.y == 0 || scaling.z == 0)
			{
				Core.LogWarning("Scaling is zero, setting the values to 1...");
				scaling = new Vector3(1, 1, 1);
			}

			bool isPrefab = Prefab.EngineIsPrefabResource(entity.ID);
			if (isPrefab)
			{
                EntityID id = Prefab.CreatePrefabEntity(entity.ID, postion, rotation, scaling);
				return new Entity(id, FindNameFromID(id));
			}
			else if (IsValidEntity(entity.ID))
			{
                EntityID id = CloneEntity(entity.ID, postion, rotation, scaling);
                return new Entity(id, FindNameFromID(id));
            }
			else
			{
				// Invalid Entity!
				return new Entity(new EntityID(), "");
			}
		}

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static EntityID CreateEntity(string name, Vector3 postion = new Vector3(), Vector3 rotation = new Vector3(), Vector3 scaling = new Vector3());

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static EntityID CloneEntity(EntityID id, Vector3 postion = new Vector3(), Vector3 rotation = new Vector3(), Vector3 scaling = new Vector3());

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool IsValidEntity(EntityID prefabid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void AddComponent(EntityID entityID, Components component);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RemoveComponent(EntityID entityID, Components component);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void DestroyEntity(EntityID entityID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static EntityID FindIDFromName(string name);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string FindNameFromID(EntityID id);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static EntityID FindParentIDFromID(EntityID id);
	}

	public class CameraSystem
	{
		// Setters
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetPosition(EntityID entityid, Vector3 newPos);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetRotation(EntityID entityid, Vector3 newRot);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetViewportSize(EntityID entityid, Vector2 newSize);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetFocalPointA(EntityID entityid, Vector3 newFocalPoint);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetFocalLength(EntityID entityid, float newFocalLength);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetPitch(EntityID entityid, float newPitch);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetYaw(EntityID entityid, float newYaw);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetRoll(EntityID entityid, float newRoll);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetFOV(EntityID entityid, float newFOV);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetNear(EntityID entityid, float newNear);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetFar(EntityID entityid, float newFar);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetLeft(EntityID entityid, float newLeft);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetRight(EntityID entityid, float newRight);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetTop(EntityID entityid, float newTop);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetBottom(EntityID entityid, float newBottom);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetAspectRatio(EntityID entityid, float newAspectRatio);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetIsPerspective(EntityID entityid, bool newIsPerspective);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetIsMainCamera(EntityID entityid, bool newIsMainCamera);


		// Getters
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetPosition(EntityID entityid, out Vector3 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetRotation(EntityID entityid, out Vector3 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetViewMatrix(EntityID entityid, out Mat4x4 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetProjectionMatrix(EntityID entityid, out Mat4x4 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetInverseViewMatrix(EntityID entityid, out Mat4x4 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetInverseProjectionMatrix(EntityID entityid, out Mat4x4 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetInverseViewProjectionMatrix(EntityID entityid, out Mat4x4 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetViewportSize(EntityID entityid, out Vector2 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetFocalPoint(EntityID entityid, out Vector3 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetFocalLength(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetPitch(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetYaw(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetRoll(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetFOV(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetNear(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetFar(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetLeft(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetRight(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetTop(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetBottom(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetAspectRatio(EntityID entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetIsPerspective(EntityID entityid, out bool output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetIsMainCamera(EntityID entityid, out bool output);
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
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void ResizeSphereCollider(EntityID entityid, float newRadius);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void ResizeBoxCollider(EntityID entityid, Vector3 newHalfExtents);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void ResizeCapsuleCollider(EntityID entityid, float newRadius, float newHelfHeight);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void AddForce(EntityID entityid, Vector3 force, ForceMode mode);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void ConstrainRotationX(EntityID entityid, bool state);
		
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void ConstrainRotationY(EntityID entityid, bool state);
		
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void ConstrainRotationZ(EntityID entityid, bool state);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetLinearVelocity(EntityID entityid, out Vector3 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetLinearVelocity(EntityID entityid, Vector3 velocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool IsCollisionEnter(EntityID entityid1, EntityID entityid2);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool IsCollisionStay(EntityID entityid1, EntityID entityid2);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool IsCollisionExit(EntityID entityid1, EntityID entityid2);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool IsTriggerEnter(EntityID entityid1, EntityID entityid2);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool IsTriggerStay(EntityID entityid1, EntityID entityid2);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool IsTriggerExit(EntityID entityid1, EntityID entityid2);
	}

	public class InputSystem
	{
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool GetKeyDown(InputKeys keycode);
	}

	public class MathF
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Sqrt(float value);
    }

	public class Random
	{
		public static int Range(int min_inclusive, int max_exclusive)
		{
			return IntRange(min_inclusive, max_exclusive);
        }

		public static float Range(float min_inclusive, float max_inclusive)
		{
			return FloatRange(min_inclusive, max_inclusive);
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static int IntRange(int min_inclusive, int max_exclusive);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static float FloatRange(float min_inclusive, float max_inclusive);
    }

	public class Time
	{
		public static readonly float deltaTime = GetDeltaTime();

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public extern static float GetDeltaTime();
	}
}
