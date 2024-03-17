using GlmSharp;

namespace TRE
{
	public class FallingObj : Entity
	{
		public Entity spawner;

		public float activeDuration;
		public float activeTimer;

		public vec3 oriRot;

		public void Update()
		{
			transform.Rotation = vec3.Zero;
			// activeTimer -= Time.deltaTime;

			// if (activeTimer > 0) return;
			// if (spawner == null) return;

			// activeTimer = activeDuration;

			// // Deactivate the object
			// SetActive(false);

			// // Get New Position for object
			// bool foundSpot = spawner.GetComponent<RandomizeFallingObjLocation>().AssignNewLocation(out vec3 newpos);

			// // Ignore if no new spot
			// if (!foundSpot) return;

			// // Reset falling object
			// transform.Position = newpos;
			// transform.Rotation = oriRot;
			// SetActive(true);
			// PhysicsSystem.SetLinearVelocity(ID, vec3.Zero);
		}
	}
}