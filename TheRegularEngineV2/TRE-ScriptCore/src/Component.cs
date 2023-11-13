using System;
using System.Collections.Generic;
using System.Linq;
using System.Management.Instrumentation;
using System.Runtime.CompilerServices;
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

    public class MeshRenderer : Component
    {
        public string Material
        {
            set
            {
                MeshRendererSystem.Engine_SetMaterialInstance(entity.ID, value);
            }
        }

        public bool Visible
        {
            get
            {
                return MeshRendererSystem.Engine_GetMaterialVisibility(entity.ID);
            }
            set
            {
                MeshRendererSystem.Engine_SetMaterialVisibility(entity.ID, value);
            }
        }

        public bool Culled
        {
            get
            {
                return MeshRendererSystem.Engine_GetMaterialCulled(entity.ID);
            }
            set
            {
                MeshRendererSystem.Engine_SetMaterialCulled(entity.ID, value);
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
        public void Play(bool once = false)
        {
            if (once)
            {
                AudioSystem.PlayOnce(entity.ID);
            }
            else
            {
                AudioSystem.Play(entity.ID);
            }
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
}
