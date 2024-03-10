using GlmSharp;

namespace TRE
{
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

		private void OnTriggerStay(/*Collider*/System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);

			if (other.CompareTag("Red")) // Moley
			{
				TransformSystem.GetPosition(this.ID, out vec3 respawnPoint);
				respawnPoint.y += RespawnHeight;
				other.GetComponent<MoleyController>().SetRespawnPoint(respawnPoint);
			}

			if (other.CompareTag("Blue")) // Holey
			{
				TransformSystem.GetPosition(this.ID, out vec3 respawnPoint);
				respawnPoint.y += RespawnHeight;
				other.GetComponent<HoleyController>().SetRespawnPoint(respawnPoint);
			}
		}
	}
}
