using System;
using System.Collections.Generic;
using System.IO.MemoryMappedFiles;
using System.Linq;
using System.Runtime.CompilerServices;
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

    public struct Entity
    {
        private string _id;
        private string _name;

        public Entity(string id, string name)
        {
            _id = id;
            _name = name;
        }
        public string GetId()
        {
            return _id;
        }
        public string GetName()
        {
            return _name;
        }

    }

    // Reference to this for what components that can be added to the entity.
    public enum Components
    {
        Mesh = 0,
        Camera = 1,
        Audio = 2,
    }

    public struct Vector3
    {
        public float x, y, z;
        public Vector3(float x, float y, float z)
        {
            this.x = x; this.y = y; this.z = z;
        }
    }

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
        public extern static void Log(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void LogWarning(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void LogError(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void LogCritical(string message);

        // Debugging Functions

        // ECS get parent

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
        public extern static void GetPosition(string id, out Vector3 output);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetPosition(string id, Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetRotation(string id, out Vector3 output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetRotation(string id, Vector3 rotation);


    }

    public class ECSManager
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static string CreateEntity(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void AddComponent(string entityID, Components component);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void RemoveComponent(string entityID, Components component);
    }

    public class CameraSystem
    {
        // Setters
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetPosition(string entityid, Vector3 newPos);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetRotation(string entityid, Vector3 newRot);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetViewportSize(string entityid, Vector2 newSize);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetFocalPointA(string entityid, Vector3 newFocalPoint);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetFocalLength(string entityid, float newFocalLength);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetPitch(string entityid, float newPitch);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetYaw(string entityid, float newYaw);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetRoll(string entityid, float newRoll);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetFOV(string entityid, float newFOV);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetNear(string entityid, float newNear);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetFar(string entityid, float newFar);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetLeft(string entityid, float newLeft);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetRight(string entityid, float newRight);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetTop(string entityid, float newTop);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetBottom(string entityid, float newBottom);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetAspectRatio(string entityid, float newAspectRatio);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetIsPerspective(string entityid, bool newIsPerspective);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetIsMainCamera(string entityid, bool newIsMainCamera);


        // Getters
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetPosition(string entityid, out Vector3 output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetRotation(string entityid, out Vector3 output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetViewMatrix(string entityid, out Mat4x4 output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetProjectionMatrix(string entityid, out Mat4x4 output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetInverseViewMatrix(string entityid, out Mat4x4 output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetInverseProjectionMatrix(string entityid, out Mat4x4 output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetInverseViewProjectionMatrix(string entityid, out Mat4x4 output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetViewportSize(string entityid, out Vector2 output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetFocalPoint(string entityid, out Vector3 output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetFocalLength(string entityid, out float output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetPitch(string entityid, out float output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetYaw(string entityid, out float output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetRoll(string entityid, out float output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetFOV(string entityid, out float output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetNear(string entityid, out float output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetFar(string entityid, out float output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetLeft(string entityid, out float output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetRight(string entityid, out float output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetTop(string entityid, out float output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetBottom(string entityid, out float output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetAspectRatio(string entityid, out float output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetIsPerspective(string entityid, out bool output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetIsMainCamera(string entityid, out bool output);

    }

    public class PhysicsSystem
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void ResizeSphereCollider(string entityid, float newradius);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void ResizeBoxCollider(string entityid, Vector3 newsize);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void AddForce(string entityid, Vector3 force);

    }


}
