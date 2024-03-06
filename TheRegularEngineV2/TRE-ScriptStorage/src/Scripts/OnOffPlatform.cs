using GlmSharp;
using System;

namespace TRE
{
	using TS = TransformSystem;

	public class OnOffPlatform : Entity
	{
		#region Constants (assigned in Start(), no changes please thanks)
		// an array of cardinal directions from 45 to 315
		int[] directions = new int[7];

		// the angle the platform must have to be in the ACTIVE position
		float platformActiveAngle;

		// the angle the platform must have to be in the INACTIVE position
		float platformInactiveAngle;

		// the direction (1 or -1) to rotate in when activating the platform
		int activationRotationDirection;
		#endregion

		// whether the platform is currently active/inactive
		bool platformState;

		// setting this to true will begin the rotation
		bool shouldRotate;

		// the angle at the beginning of the rotation
		float rotateStartAngle;

		// the angle to have at the end of the rotation
		float rotateEndAngle;

		// determines which way to rotate, value is either 1 or -1
		int rotationDirection;

		// the change in angle per unit(frame?) of time in a rotation
		const float rotationSpeed = 40.0f;

		float timer;
		vec3 originalPosition;

		public void Start()
		{
			// I can't do just the line below... :(
			// int[] directions = { 45, 90, 135, 180, 225, 270, 315 };
			for (int i = 0; i < 7; ++i)
				directions[i] = 45 * (i + 1);

			TS.GetRotation(this.ID, out vec3 initialRot);
			float initialRotY = NiceRotationAngle(initialRot.y);

			// set constants based on initialRotY
			if (initialRotY == 0)
			{
				platformInactiveAngle = 0.0f;
				platformActiveAngle = 90.0f;
				activationRotationDirection = 1;
				// use the z axis to rotate
			}
			else if (initialRotY == 90)
			{
				// FUCKING GIMBAL LOCK?!?
			}
			else if (initialRotY == 180)
			{
				platformInactiveAngle = 0.0f;
				platformActiveAngle = -90.0f;
				activationRotationDirection = -1;
				// use the z axis to rotate
			}
			else if (initialRotY == 270)
			{
				// FUCKING GIMBAL LOCK?!?
			}

			platformState = false;
			timer = 0;
			TS.GetPosition(this.ID, out vec3 pos);
			originalPosition = pos;
		}

		public void Update()
		{
			if (InputSystem.GetKeyPress(InputKeys.LeftBracket))
				SetPlatformState(false);
			if (InputSystem.GetKeyPress(InputKeys.RightBracket))
				SetPlatformState(true);

			timer += Time.deltaTime;

			// TrembleZ();

			if (shouldRotate)
				PerformRotationZ();
		}

		private bool IsInRange(float angle, float direction)
		{
			const float rangeInDegrees = 5.0f;
			if (direction - rangeInDegrees <= angle && angle <= direction + rangeInDegrees)
				return true;
			else
				return false;
		}

		// a 'nice' rotation is a rotation in degrees of only the following:
		// { 0, 45, 90, 135, 180, 225, 270, 315 }
		private float NiceRotationAngle(float rotYInDegrees)
		{
			while (rotYInDegrees < 0)
				rotYInDegrees += 360;
			while (rotYInDegrees > 360)
				rotYInDegrees -= 360;

			for (int i = 0; i < directions.Length; ++i)
			{
				if (IsInRange(rotYInDegrees, directions[i]))
					return directions[i];
			}
			return 0;
		}

		public bool GetPlatformState()
		{
			return platformState;
		}

		public void SetPlatformState(bool state)
		{
			// do nothing if the platform already matches the given state
			if (!shouldRotate && platformState == state)
			{
				return;
			}

			// send the "signal" that a rotation should take place
			shouldRotate = true;

			// assign variables based on the constants set in Start()
			if (state)
			{
				// go from inactive to active
				rotateStartAngle = platformInactiveAngle;
				rotateEndAngle = platformActiveAngle;

				rotationDirection = activationRotationDirection;
			}
			else
			{
				// go from active to inactive
				rotateStartAngle = platformActiveAngle;
				rotateEndAngle = platformInactiveAngle;

				// the direction should be the OPPOSITE of the direction needed for activation
				rotationDirection = -activationRotationDirection;
			}
		}

		void PerformRotationZ()
		{
			TS.GetRotation(this.ID, out vec3 rot);
			float amountComplete = InverseLerp(rotateStartAngle, rotateEndAngle, rot.z);

			if (amountComplete < 1)
			{
				rot.z += rotationSpeed * Time.deltaTime * rotationDirection;
				TS.SetRotation(this.ID, rot);
			}
			else // if (amountComplete >= 1)
			{
				shouldRotate = false;
				float angle = NiceRotationAngle(rot.z);
				platformState = angle == platformActiveAngle;
			}
		}

		/*
		void Swap(ref float a, ref float b)
		{
			// float temp = a;
			// a = b;
			// b = temp;
			(b, a) = (a, b);
		}
		*/

		float InverseLerp(float start, float end, float output)
		{
			// r = p + (t)(q-p) <- Lerp formula
			// r - p = (t)(q-p)
			// t = (r - p) / (q - p)
			return (output - start) / (end - start);
		}

		void TrembleZ()
		{
			const float amplitude = 0.125f;
			const float frequency = 60.0f;

			TS.GetPosition(this.ID, out vec3 pos);
			pos.z = originalPosition.z + amplitude * MathF.Sin(timer * frequency);
			TS.SetPosition(this.ID, pos);
		}
	}
}
