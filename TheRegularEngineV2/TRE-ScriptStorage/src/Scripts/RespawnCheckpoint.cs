using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;

namespace TRE
{
    using PS = PhysicsSystem;
    public class RespawnCheckpoint : Entity
    {
        private Entity Player1;
        private Entity Player2;

        private Entity RespawnPoint1;

        private vec3 RespawnPositionBlue = new vec3(0, 0, 0);
        private vec3 RespawnPositionRed = new vec3(0, 0, 0);

        public void Start()
        {
            Player1 = ECSManager.FindEntityByName("Holey");
            Player2 = ECSManager.FindEntityByName("Moley");
            RespawnPoint1 = ECSManager.FindEntityByName("RespawnCollider1");
            Debug.Log("Respawn Checkpoint Start Function");
        }

        public void OnCreate()
        {

        }

        private void OnTriggerStay(/*Collider*/System.UInt64 otherID)
        {

        }

        private void OnCollisionStay(System.UInt64 otherID)
        {

        }

        public void Update()
        {
            if (ECSManager.IsValidEntity(RespawnPoint1.ID) && IsInsideTrigger(RespawnPoint1))
            {
                TransformSystem.GetPosition(RespawnPoint1.ID, out vec3 RespawnPositionBlue);
            }
        }

        private bool IsInsideTrigger(Entity entity)
        {
            return ECSManager.IsValidEntity(entity.ID) && (PS.IsTriggerEnter(Player1.ID, entity.ID) || PS.IsTriggerStay(Player1.ID, entity.ID));
        }
    }
}