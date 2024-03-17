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

			TransformSystem.GetPosition(this.ID, out vec3 respawnPoint);
			respawnPoint.y += RespawnHeight;

			if (other.CompareTag("Red")) // Moley
				other.GetComponent<MoleyController>().SetRespawnPoint(respawnPoint);

			if (other.CompareTag("Blue")) // Holey
				other.GetComponent<HoleyController>().SetRespawnPoint(respawnPoint);
		}
	}
}
