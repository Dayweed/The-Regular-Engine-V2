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
	public class Entity
	{
		public string id;				// Can hold id of entity or id of prefab resource
		public string name;
		public Parenting parenting;
		public Transform transform;

        public Entity(string _name, string _id = "")
		{
			name = _name;
			id = _id;

			parenting = new Parenting(id);
			TransformSystem.GetPosition(id, out Vector3 pos);
			TransformSystem.GetRotation(id, out Vector3 rot);
            transform = new Transform(id, pos, rot, new Vector3(1,1,1));
        }

        public void Rename(string _name)
		{
			name = _name;
            EngineRename(id, name);
        }

        public void SetActive(bool active)
		{
			EngineSetActive(id, active);
        }

        public bool GetActive()
		{
            return EngineGetActive(id);
        }

        public void SetTag(string tag)
		{
            EngineSetTag(id, tag);
        }

        public string GetTag()
		{
            return EngineGetTag(id);
        }

        public bool CompareTag(string otherTag)
		{
            return EngineCompareTag(id, otherTag);
        }

        // Private binded calls
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void EngineRename(string id, string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void EngineSetActive(string id, bool active);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool EngineGetActive(string id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void EngineSetTag(string id, string tag);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string EngineGetTag(string id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool EngineCompareTag(string id, string otherTag);
    }

	public struct Prefab
	{
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool EngineIsPrefabResource(string id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string CreatePrefabEntity(string prefabid, Vector3 postion = new Vector3(), Vector3 rotation = new Vector3(), Vector3 scaling = new Vector3());
    }

	public struct Parenting
	{
        private string id;

        public Parenting(string myID)
		{
			id = myID;
        }

        public void SetParent(Entity _parent)
        {
			EngineParentSetParent(id, _parent.id);
        }

        public Entity GetParent()
        {
            string parentID = ECSManager.FindParentIDFromID(id);
            string parentName = ECSManager.FindNameFromID(parentID);
            Entity parent = new Entity(parentName, parentID);
            return parent;
        }

        public void RemoveParent()
        {
            EngineParentRemoveParent(id);
        }

		public void AddChild(Entity child)
		{
			EngineParentAddChild(id, child.id);
		}

		public void RemoveChild(Entity child)
		{
			EngineParentRemoveChild(id, child.id);
		}

        public Entity GetChild(int _index)
		{
			string childID = EngineGetChildID(id, _index);
			string childName = ECSManager.FindNameFromID(childID);
            Entity child = new Entity(childName, childID);
			return child;
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string EngineGetChildID(string _id, int _index);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string EngineParentSetParent(string _id, string _parent_id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string EngineParentRemoveParent(string _id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string EngineParentAddChild(string _id, string _child_id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string EngineParentRemoveChild(string _id, string _child_id);
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
			return 0.0f;
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
		private string id;
		public Vector3 position, rotation, scale;

		public Transform(string _id = "", Vector3 _pos = new Vector3(), Vector3 _rot = new Vector3(), Vector3 _sca = new Vector3())
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
		public void transformDemo(string id)
		{
			Vector3 test = new Vector3(0, 0, 0);
			GetRotation(id, out test);

			test.x += 1;

			SetRotation(id, test);
		}

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetPosition(string id, out Vector3 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetPosition(string id, Vector3 position);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetRotation(string id, out Vector3 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetRotation(string id, Vector3 rotation);
	}

	public class ECSManager
	{
		public static Entity Instantiate(Entity entity, Vector3 postion = new Vector3(), Vector3 rotation = new Vector3(), Vector3 scaling = new Vector3())
		{
			// Ensure scaling is not zero, since Vector3 does not allow const version currently and must compile-time const
			if (scaling.x == 0 || scaling.y == 0 || scaling.z == 0)
			{
				Core.LogWarning("Scaling is zero, setting the values to 1...");
				scaling = new Vector3(1, 1, 1);
			}

			bool isPrefab = Prefab.EngineIsPrefabResource(entity.id);
			if (isPrefab)
			{
				string id = Prefab.CreatePrefabEntity(entity.id, postion, rotation, scaling);
				return new Entity(FindNameFromID(id), id);
			}
			else if (IsValidEntity(entity.id))
			{
				string id = CloneEntity(entity.id, postion, rotation, scaling);
                return new Entity(FindNameFromID(id), id);
            }
			else
			{
				// Invalid Entity!
				return new Entity("");
			}
		}

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string CreateEntity(string name, Vector3 postion = new Vector3(), Vector3 rotation = new Vector3(), Vector3 scaling = new Vector3());

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string CloneEntity(string id, Vector3 postion = new Vector3(), Vector3 rotation = new Vector3(), Vector3 scaling = new Vector3());

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool IsValidEntity(string prefabid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void AddComponent(string entityID, Components component);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RemoveComponent(string entityID, Components component);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string FindIDFromName(string name);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string FindNameFromID(string id);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string FindParentIDFromID(string id);
	}

	public class CameraSystem
	{
		// Setters
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetPosition(string entityid, Vector3 newPos);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetRotation(string entityid, Vector3 newRot);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetViewportSize(string entityid, Vector2 newSize);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetFocalPointA(string entityid, Vector3 newFocalPoint);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetFocalLength(string entityid, float newFocalLength);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetPitch(string entityid, float newPitch);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetYaw(string entityid, float newYaw);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetRoll(string entityid, float newRoll);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetFOV(string entityid, float newFOV);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetNear(string entityid, float newNear);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetFar(string entityid, float newFar);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetLeft(string entityid, float newLeft);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetRight(string entityid, float newRight);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetTop(string entityid, float newTop);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetBottom(string entityid, float newBottom);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetAspectRatio(string entityid, float newAspectRatio);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetIsPerspective(string entityid, bool newIsPerspective);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetIsMainCamera(string entityid, bool newIsMainCamera);


		// Getters
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetPosition(string entityid, out Vector3 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetRotation(string entityid, out Vector3 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetViewMatrix(string entityid, out Mat4x4 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetProjectionMatrix(string entityid, out Mat4x4 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetInverseViewMatrix(string entityid, out Mat4x4 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetInverseProjectionMatrix(string entityid, out Mat4x4 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetInverseViewProjectionMatrix(string entityid, out Mat4x4 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetViewportSize(string entityid, out Vector2 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetFocalPoint(string entityid, out Vector3 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetFocalLength(string entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetPitch(string entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetYaw(string entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetRoll(string entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetFOV(string entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetNear(string entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetFar(string entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetLeft(string entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetRight(string entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetTop(string entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetBottom(string entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetAspectRatio(string entityid, out float output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetIsPerspective(string entityid, out bool output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetIsMainCamera(string entityid, out bool output);
	}

	public class PhysicsSystem
	{
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void ResizeSphereCollider(string entityid, float newradius);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void ResizeBoxCollider(string entityid, Vector3 newsize);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void AddForce(string entityid, Vector3 force);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void ConstrainRotationX(string entityid, bool state);
		
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void ConstrainRotationY(string entityid, bool state);
		
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void ConstrainRotationZ(string entityid, bool state);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void GetLinearVelocity(string entityid, out Vector3 output);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SetLinearVelocity(string entityid, Vector3 velocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool IsCollisionEnter(string entityid1, string entityid2);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool IsCollisionStay(string entityid1, string entityid2);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool IsCollisionExit(string entityid1, string entityid2);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool IsTriggerEnter(string entityid1, string entityid2);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool IsTriggerStay(string entityid1, string entityid2);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool IsTriggerExit(string entityid1, string entityid2);
	}

	public class InputSystem
	{
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool GetKeyDown(InputKeys keycode);
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
