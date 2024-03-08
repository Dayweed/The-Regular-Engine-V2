using GlmSharp;
using System;

namespace TRE
{
	using TS = TransformSystem;

	public class OnOffPlatform : Entity
	{
		#region Constants (assigned in Start(), no changes please thanks)
		// determines if it's a left/right platform or forward/backward platform
		// the forward/backward is the "opposite" type of platform, rotates on x axis
		// left/right has bigger scale on z axis than x axis
		// forward/backward has bigger scale on x axis than z axis
		bool isOppositePlatformType;

		// an array of cardinal directions from 45 to 315
		int[] directions = new int[7];

		// the angle the platform must have to be in the ACTIVE position
		float platformActiveAngle;

		// the angle the platform must have to be in the INACTIVE position
		float platformInactiveAngle;

		// the direction (1 or -1) to rotate in when activating the platform
		int activationRotationDirection;

		// the rotation function to use
		Action RotateFunction;

		// the tremble function to use
		Action TrembleFunction;
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
			// get the actual platform(the child)'s scale to determine what type of platform it is
			vec3 platformScale = parenting.GetChild(0).transform.Scale;
			isOppositePlatformType = platformScale.x > platformScale.z;

			// I can't do just the line below... :(
			// int[] directions = { 45, 90, 135, 180, 225, 270, 315 };
			for (int i = 0; i < 7; ++i)
				directions[i] = 45 * (i + 1);

			TS.GetRotation(this.ID, out vec3 initialRot);

			// sometimes the rotation that this entity has is really weird
			// this part is to figure out whether its either a 0 or 180 rotation on the y axis
			// if so, FIX THAT NONSENSE BEFORE ROTATING!!!
			float actualRotY = 0;
			if (HasNoRotation(initialRot))
			{
				actualRotY = 0;
				transform.Rotation = vec3.Zero;
			}
			else if (IsCursedRotation(initialRot) || HasFlippedRotation(initialRot))
			{
				actualRotY = 180;
				transform.Rotation = new vec3(0, 180, 0);
			}

			// set constants based on actualRotY
			if (actualRotY == 0)
			{
				platformInactiveAngle = 0.0f;
				platformActiveAngle = 90.0f;
				activationRotationDirection = 1;
			}
			else if (actualRotY == 180)
			{
				platformInactiveAngle = 0.0f;
				platformActiveAngle = -90.0f;
				activationRotationDirection = -1;

				// i have no clue why removing the negatives work but it does
				if (isOppositePlatformType)
				{
					platformActiveAngle *= -1;
					activationRotationDirection *= -1;
				}
			}

			// assign corresponding behaviours/functions
			if (isOppositePlatformType)
			{
				RotateFunction = PerformRotationX;
				TrembleFunction = TrembleX;
			}
			else
			{
				RotateFunction = PerformRotationZ;
				TrembleFunction = TrembleZ;
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

			// TrembleFunction();

			if (shouldRotate)
				RotateFunction();
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

		bool IsInRange(float angle, float direction)
		{
			const float rangeInDegrees = 5.0f;
			if (direction - rangeInDegrees <= angle && angle <= direction + rangeInDegrees)
				return true;
			else
				return false;
		}

		// a 'nice' rotation is a rotation in degrees of only the following:
		// { 0, 45, 90, 135, 180, 225, 270, 315 }
		float NiceRotationAngle(float rotYInDegrees)
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
				platformState = IsInRange(rot.z, platformActiveAngle);
			}
		}

		void PerformRotationX()
		{
			TS.GetRotation(this.ID, out vec3 rot);
			float amountComplete = InverseLerp(rotateStartAngle, rotateEndAngle, rot.x);

			if (amountComplete < 1)
			{
				rot.x += rotationSpeed * Time.deltaTime * rotationDirection;
				TS.SetRotation(this.ID, rot);
			}
			else // if (amountComplete >= 1)
			{
				shouldRotate = false;
				platformState = IsInRange(rot.x, platformActiveAngle);
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

		void TrembleX()
		{
			const float amplitude = 0.125f;
			const float frequency = 60.0f;

			TS.GetPosition(this.ID, out vec3 pos);
			pos.x = originalPosition.x + amplitude * MathF.Sin(timer * frequency);
			TS.SetPosition(this.ID, pos);
		}

		void print(string str)
		{
			Console.Write("[{0}]\t{1}", this.name, str + "\n");
		}

		// sometimes (0, 0, 0) becomes (0, -0, 0) :_)
		bool HasNoRotation(vec3 rot)
		{
			return IsVec3Equal(rot, vec3.Zero);
		}

		// preparing for the event that (0, 180, 0) becomes (0, -180, 0)
		bool HasFlippedRotation(vec3 rot)
		{
			return IsVec3Equal(rot, new vec3(0, 180, 0));
		}

		// sometimes (0, 180, 0) becomes (-180, 0, -180) :_)
		bool IsCursedRotation(vec3 rot)
		{
			return IsVec3Equal(rot, new vec3(-180, 0, -180));
		}

		bool IsVec3Equal(vec3 a, vec3 b)
		{
			const float EPSILON = 0.001f;
			float diffX = a.x - b.x;
			float diffY = a.y - b.y;
			float diffZ = a.z - b.z;

			bool isDiffZeroX = -EPSILON <= diffX && diffX <= EPSILON;
			bool isDiffZeroY = -EPSILON <= diffY && diffY <= EPSILON;
			bool isDiffZeroZ = -EPSILON <= diffZ && diffZ <= EPSILON;

			return isDiffZeroX && isDiffZeroY && isDiffZeroZ;
		}
	}
}
