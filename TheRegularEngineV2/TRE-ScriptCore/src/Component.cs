using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TRE
{
    public abstract class Component
    {
        public Entity entity { get; internal set; }
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
                TransformSystem.GetRotation(entity.ID, out Vector3 rot);
                return rot;
            }
            set
            {
                TransformSystem.SetRotation(entity.ID, value);
            }
        }

        public Vector3 Scale
        {
            get
            {
                TransformSystem.GetScaling(entity.ID, out Vector3 scale);
                return scale;
            }
            set
            {
                TransformSystem.SetScaling(entity.ID, value);
            }
        }

        public Transform()
        {
            Position = new Vector3();
            Rotation = new Vector3();
            Scale = new Vector3(1, 1, 1);
        }

        public Transform(Vector3 position, Vector3 rotation, Vector3 scale)
        {
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
