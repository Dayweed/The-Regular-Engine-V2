using System;
using System.Collections.Generic;
using System.Linq;
using System.Management.Instrumentation;
using System.Text;
using System.Threading.Tasks;

using GlmSharp;

namespace TRE
{

    // using EntityID = System.UInt64;

    public abstract class Component
    {
        public Entity entity { get ; set; }
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
                if (entity.ID == 0)
                {
                    return new vec3(0, 0, 0);
                }
                TransformSystem.Engine_GetRotation(entity.ID, out vec3 rot);
                return rot;
            }
            set
            {
                if (entity.ID == 0)
                {
                    return;
                }
                TransformSystem.Engine_SetRotation(entity.ID, value);
            }
        }

        public vec3 Scale
        {
            get
            {
                if (entity.ID == 0)
                {
                    return new vec3(1, 1, 1);
                }
                TransformSystem.Engine_GetScaling(entity.ID, out vec3 scale);
                return scale;
            }
            set
            {
                if (entity.ID == 0)
                {
                    return;
                }
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
