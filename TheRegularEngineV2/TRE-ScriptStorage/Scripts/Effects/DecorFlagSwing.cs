using GlmSharp;
using System;

namespace TRE
{
	using TS = TransformSystem;

	// Ensure this script is attached to the Decor_Flag's PIVOT (which must be the
	// Decor_Flag's parent), not the Decor_Flag itself!

	// Given a Decor_Flag with a default scale, its pivot should be placed ~ 3.5 units
	// higher than the Decor_Flag itself.

	class DecorFlagSwing : Entity
	{
		vec3 initialRotation;
		Action swingFunction;

		// All of the variables below are assigned in InitializeDefaultBehaviour()
		public float timer;
		bool pause;
		public float amplitude;
		public float period;
		/// <summary>Must only have the value of 1 or -1.
		/// Used to make things move/rotate in the opposite way.</summary>
		public int dir;

		public void Start()
		{
			// initialRotation = entity.transform.Rotation;
			// the line above was cursed (causing crashes) so here's this instead:
			TS.GetRotation(this.ID, out vec3 temp);
			initialRotation = temp;

			InitializeDefaultBehaviour();

			// if (name == "some other thing")
			//     make other modifications specific to an entity here...;
			if (Scene.GetSceneName() == "Start")
			{
				// make the other flag a little bit further along in its swing
				if (name == "Decor_Flag_Pivot_2")
				{
					timer = 1.0f;
					return;
				}
			}

			// having a "!" in the name will make it swing on the z axis instead
			if (name.Contains("!"))
				swingFunction = SwingOnZ;

			// if the name has a digit at the end, let it be the change in the initial rotation
			string nameLastChar = name.Substring(name.Length - 1);
			if (int.TryParse(nameLastChar, out int result))
				timer = result;
		}

		public void Update()
		{
			if (pause) return;

			timer += Time.deltaTime;
			swingFunction();
		}

		public void Pause()
		{
			pause = true;
		}

		public void Resume()
		{
			pause = false;
		}

		void InitializeDefaultBehaviour()
		{
			timer = 0;
			pause = false;
			amplitude = 30.0f;
			period = 1.0f;
			dir = 1;
			swingFunction = SwingOnX;
		}

		void SwingOnX()
		{
			TS.GetRotation(this.ID, out vec3 rot);
			float value = initialRotation.x + amplitude * MathF.Sin(period * timer) * dir;
			rot.x = value;
			TS.SetRotation(this.ID, rot);
		}

		void SwingOnZ()
		{
			TS.GetRotation(this.ID, out vec3 rot);
			float value = initialRotation.z + amplitude * MathF.Sin(period * timer) * dir;
			rot.z = value;
			TS.SetRotation(this.ID, rot);
		}
	}
}
