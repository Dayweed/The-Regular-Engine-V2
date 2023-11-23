using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;
using Coroutine;

namespace TRE
{
    using PS = PhysicsSystem;

    /*
     *
     * HOW RESPAWNING WORKS:
     *  For this script, the sole purpose is to be placed on to multiple colliders throughout the level
     *  and when the player enters the collider, the player's position is set to the position of the the specified spawn point.
     *  
     *  NOTE: For the respawning logic, it will be handled in the player's controller script.
     *
     *
     */
    public class RespawnCheckpoint : Entity
    {

        private Dictionary<string ,vec3> RespawnPoints = new Dictionary<string, vec3>();
        public void Start()
        {
            Debug.Log("Respawn Checkpoint Start Function");

            // Configure the respawn positions for all the level checkpoints

            // Tutorial / Level 0
            RespawnPoints.Add("RespawnPoint1_L0", new vec3(112.112f, 45.082f, -68.054f));
            RespawnPoints.Add("RespawnPoint2_L0", new vec3(112.112f, 39.310f, -99.054f));
            RespawnPoints.Add("RespawnPoint3_L0", new vec3(118.834f, 35.078f, -148.931f));
            RespawnPoints.Add("RespawnPoint4_L0", new vec3(114.112f, 39.310f, -281.054f));
            RespawnPoints.Add("RespawnPoint5_L0", new vec3(114.112f, 39.310f, -310.054f));
            RespawnPoints.Add("RespawnPoint6_L0", new vec3(60.112f, 70.337f, -337.054f));
            RespawnPoints.Add("RespawnPoint7_L0", new vec3(85.112f, 71.167f, -391.054f));

            // Level 1
            // adjust the spawning locations accordingly to what is desired.
            RespawnPoints.Add("RespawnPoint1_L1", new vec3(-16.566f, 50.011f, -113.158f));
            RespawnPoints.Add("RespawnPoint2_L1", new vec3(-20.566f, 50.011f, -162.158f));
            RespawnPoints.Add("RespawnPoint3_L1", new vec3(-3.988f, 75.035f, -217.881f));
            RespawnPoints.Add("RespawnPoint4_L1", new vec3(42.012f, 75.035f, -202.881f));
            RespawnPoints.Add("RespawnPoint5_L1", new vec3(104.012f, 75.035f, -202.881f));
            RespawnPoints.Add("RespawnPoint6_L1", new vec3(-179.012f, 74.035f, -278.881f));
            RespawnPoints.Add("RespawnPoint7_L1", new vec3(369.012f, 103.035f, -282.881f));
            RespawnPoints.Add("RespawnPoint8_L1", new vec3(528.012f, 103.035f, -182.881f));
            RespawnPoints.Add("RespawnPoint9_L1", new vec3(604.012f, 152.035f, -170.881f));
            RespawnPoints.Add("RespawnPoint10_L1", new vec3(921.012f, 152.035f, -170.881f));
        }

        private void OnTriggerStay(/*Collider*/System.UInt64 otherID)
        {
            
            Entity other = new Entity(otherID);
            

            if (other.CompareTag("Red")) // Moley
            {
                //Debug.Log(other.name + " got triggered!");
                // here we will set the player's respawn position to the position of the checkpoint that is matched.
                other.GetComponent<MoleyController>().SetRespawnPoint(RespawnPoints[this.name]);
            }

            if (other.CompareTag("Blue")) // Holey
            {
                //Debug.Log(other.name + " got triggered!");
                other.GetComponent<HoleyController>().SetRespawnPoint(RespawnPoints[this.name]);
            }
        }

        private void OnCollisionStay(System.UInt64 otherID)
        {

        }

        public void Update()
        {
            
        }

    }
}