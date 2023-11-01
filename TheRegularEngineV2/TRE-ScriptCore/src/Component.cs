using System;
using System.Collections.Generic;
using System.Linq;
using System.Management.Instrumentation;
using System.Text;
using System.Threading.Tasks;

namespace TRE
{

    // using EntityID = System.UInt64;

    public abstract class Component
    {
        public Entity entity { get ; set; }
    }

    public class Transform : Component
    {
        public Vector3 Position
        {
            get
            {
                TransformSystem.GetPosition(entity.ID, out Vector3 pos);
                return pos;
            }
            set
            {
                TransformSystem.SetPosition(entity.ID, value);
            }
        }

        public Vector3 Rotation
        {
            get
            {
                if (entity.ID == 0)
                {
                    return new Vector3(0, 0, 0);
                }
                TransformSystem.GetRotation(entity.ID, out Vector3 rot);
                return rot;
            }
            set
            {
                if (entity.ID == 0)
                {
                    return;
                }
                TransformSystem.SetRotation(entity.ID, value);
            }
        }

        public Vector3 Scale
        {
            get
            {
                if (entity.ID == 0)
                {
                    return new Vector3(1, 1, 1);
                }
                TransformSystem.GetScaling(entity.ID, out Vector3 scale);
                return scale;
            }
            set
            {
                if (entity.ID == 0)
                {
                    return;
                }
                TransformSystem.SetScaling(entity.ID, value);
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
        public Transform(Vector3 position, Vector3 rotation, Vector3 scale)
        {
            // Set the default values
            Position = position;
            Rotation = rotation;
            Scale = scale;
        }
    }

    public class Camera : Component
    {
        // use transform component as template
        // add Camera variables here
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
                return RigidBodySystem.GetGravity(entity.ID);
            }
            set
            {
                RigidBodySystem.SetGravity(entity.ID, value);
            }
        }
        public bool isKinematic
        {
            get
            {                
                return RigidBodySystem.GetKinematic(entity.ID);
            }
            set
            {
                RigidBodySystem.SetKinematic(entity.ID, value);
            }
        }
    }

    public class BoxCollider : Component
    {

    }

    public class SphereCollider : Component
    {

    }

    public class CapsuleCollider : Component
    {

    }

    public class MeshRenderer : Component
    {

    }
}
