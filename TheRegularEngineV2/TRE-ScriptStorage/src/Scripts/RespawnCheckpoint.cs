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

		public float RespawnHeight = 5.0f;


		public void Start()
		{
		}

		private void OnTriggerStay(/*Collider*/System.UInt64 otherID)
		{

			Entity other = new Entity(otherID);


			if (other.CompareTag("Red")) // Moley
			{
				vec3 respawnPoint = vec3.Zero;
				TransformSystem.GetPosition(this.ID, out respawnPoint);
				respawnPoint.y += RespawnHeight;
				other.GetComponent<MoleyController>().SetRespawnPoint(respawnPoint);
			}

			if (other.CompareTag("Blue")) // Holey
			{
				vec3 respawnPoint = vec3.Zero;
				TransformSystem.GetPosition(this.ID, out respawnPoint);
				respawnPoint.y += RespawnHeight;
				other.GetComponent<HoleyController>().SetRespawnPoint(respawnPoint);
			}
		}

	}
}