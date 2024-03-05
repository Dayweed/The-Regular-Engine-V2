using GlmSharp;
using System;

namespace TRE
{
	using TS = TransformSystem;

	class OnOffPlatform : Entity
	{
		bool platformState;
		bool shouldRotate;

		float timer;
		const float defaultDuration = 3.0f;
		float duration;
		float rotateStartTime;
		float rotateStart;
		float rotateEnd;

		// for convenience's sake
		Action<string> print = Console.WriteLine;

		public void Start()
		{
			platformState = false;
			shouldRotate = false;

			timer = 0.0f;
			duration = defaultDuration;
			rotateStartTime = 0.0f;
			rotateStart = -90.0f;
			rotateEnd = 0.0f;
		}

		public void Update()
		{
			if (InputSystem.GetKeyPress(InputKeys.KPAdd))
				SetPlatformState(true);
			if (InputSystem.GetKeyPress(InputKeys.KPEnter))
				SetPlatformState(false);

			timer += Time.deltaTime;

			if (shouldRotate)
				PerformRotation();
		}

		public bool GetPlatformState()
		{
			return platformState;
		}

		public void SetPlatformState(bool state)
		{
			// do nothing if the platform already matches the given state
			if (platformState == state)
			{
				print("already there bro. :)");
				return;
			}

			// continue as normal if incoming state is the one being worked on
			// this is checked by seeing if the platform has reached its goal and what its goal is
			bool workingOnActive = platformState != state && rotateEnd == 0;
			bool workingOnInactive = platformState != state && rotateEnd == -90.0f;
			if (shouldRotate && (workingOnActive || workingOnInactive))
			{
				print("bro i'm working on that already");
				print("workingOnActive: " + workingOnActive.ToString());
				print("workingOnInactive: " + workingOnInactive.ToString());
				print("");
				return;
			}

			// let the rotation know what time it began
			rotateStartTime = timer;

			// if it was ALREADY rotating beforehand, start from its current rotation
			if (shouldRotate)
			{
				TS.GetRotation(this.ID, out vec3 currRot);
				float currRotAngle = currRot.z;
				const float defaultRotSpeed = 90 / defaultDuration;

				if (!state) // <- should that just be `state`...hmm...
				{
					print("trying to go UP from middle of rotation");
					// start from current, end at 0
					rotateStart = currRotAngle;
					rotateEnd = 0.0f;
					// should be the same speed as normal
					duration = Math.Abs(rotateEnd - rotateStart) / defaultRotSpeed;
				}
				else
				{
					print("trying to go DOWN from middle of rotation");
					// start from current, end at -90
					rotateStart = currRotAngle;
					rotateEnd = -90.0f;
					// should be the same speed as normal
					duration = Math.Abs(rotateEnd - rotateStart) / defaultRotSpeed;
				}
			}
			else // NOT currently rotating
			{
				shouldRotate = true;

				if (state)
				{
					print("Rotating UP from standstill");
					// start from -90, end at 0
					rotateStart = -90.0f;
					rotateEnd = 0.0f;
					duration = defaultDuration;
				}
				else
				{
					print("Rotating DOWN from standstill");
					// start from 0, end at -90
					rotateStart = 0.0f;
					rotateEnd = -90.0f;
					duration = defaultDuration;
				}
			}
		}

		void PerformRotation()
		{
			float value = (timer - rotateStartTime) / duration;
			if (0 <= value && value < 1)
			{
				float rotAngle = MathF.Lerp(rotateStart, rotateEnd, value);
				TS.SetRotation(this.ID, new vec3(0, 0, rotAngle));
			}
			else // when the rotation is complete
			{
				shouldRotate = false;
				// if the platform has no rotation, it's active
				platformState = rotateEnd == 0;
			}
		}
	}
}
