using GlmSharp;
using System;

namespace TRE
{
	using TS = TransformSystem;

	public class OnOffPlatform : Entity
	{
		// assigned in Start(), no changes please thanks
		#region Constants
		/// <summary>
		/// <para>Determines if it's a left/right platform or forward/backward platform.</para>
		/// The forward/backward is the "opposite" type of platform, rotates on the x axis.<br/>
		/// Left/right has a bigger scale on z axis than x axis.<br/>
		/// Forward/backward has a bigger scale on x axis than z axis.
		/// </summary>
		bool isOppositePlatformType;

		/// <summary>An array of cardinal directions from 45 to 315</summary>
		int[] directions = new int[7];

		/// <summary>The angle the platform must have to be in the ACTIVE position.</summary>
		float platformActiveAngle;

		/// <summary>The angle the platform must have to be in the INACTIVE position.</summary>
		float platformInactiveAngle;

		/// <summary>The direction (1 or -1) to rotate in when activating the platform.</summary>
		int activationRotationDirection;

		/// <summary>The rotation function to use. (e.g. Rotate*())</summary>
		Action RotateFunction;

		/// <summary>The tremble function to use. (e.g. Tremble*())</summary>
		Action TrembleFunction;

		/// <summary>The change in angle per unit(frame?) of time in a rotation when going up.</summary>
		const float goUpRotationSpeed = 120.0f;

		/// <summary>The change in angle per unit(frame?) of time in a rotation when going down.</summary>
		const float goDownRotationSpeed = 40.0f;

		/// <summary>The total amount of time in seconds for the platform to tremble before rotating.</summary>
		const float trembleDuration = 1.0f; // 0.5f for FAST
		#endregion

		#region Variables
		/// <summary>Whether the platform was active or inactive at standstill.</summary>
		bool platformState;

		/// <summary>Setting this to true will begin the rotation.</summary>
		bool shouldRotate;

		/// <summary>The angle at the beginning of the rotation.</summary>
		float rotateStartAngle;

		/// <summary>The angle to have at the end of the rotation.</summary>
		float rotateEndAngle;

		/// <summary>Determines which way to rotate, value is either 1 or -1.</summary>
		int rotationDirection;

		/// <summary>The amount of time in seconds that has passed since this entity was instantiated.</summary>
		float globalTimer;

		/// <summary>The position of the platform at Start() time.</summary>
		vec3 originalPosition;

		/// <summary>Setting this to true will begin the trembling.</summary>
		bool shouldTremble;

		/// <summary>The number of seconds this platform has been trembling for.</summary>
		float trembleTimer;

		/// <summary>Used by the child's `IsColliding` script to tell the parent its result.</summary>
		public bool isCollidingWithPlayer = false;
		#endregion

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
				//transform.Rotation = vec3.Zero;
			}
			else if (IsCursedRotation(initialRot) || HasFlippedRotation(initialRot))
			{
				actualRotY = 180;
				//transform.Rotation = new vec3(0, 180, 0);
			}

			// set constants based on actualRotY
			if (actualRotY == 0)
			{
				platformInactiveAngle = isOppositePlatformType ? transform.Rotation.x : transform.Rotation.z;
				platformActiveAngle = platformInactiveAngle + 90f;
				activationRotationDirection = 1;
			}
			else if (actualRotY == 180)
			{
				platformInactiveAngle = isOppositePlatformType ? transform.Rotation.x : transform.Rotation.z;
				platformActiveAngle = platformInactiveAngle + (isOppositePlatformType ? 1 : -1) * 90f;
				activationRotationDirection = -1;

				// i have no clue why removing the negatives work but it does
				if (isOppositePlatformType)
				{
					//platformActiveAngle *= -1;
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
			shouldRotate = false;
			globalTimer = 0;
			TS.GetPosition(this.ID, out vec3 pos);
			originalPosition = pos;
			shouldTremble = false;
			trembleTimer = 0;
		}

		public void Update()
		{
			globalTimer += Time.deltaTime;

			if (shouldTremble)
				TrembleFunction();

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
				return;

			// ignore requests while trembling
			if (shouldTremble)
				return;

			// only do trembling when rotating from standstill
			if (!shouldRotate)
			{
				shouldTremble = true;
				trembleTimer = 0;
			}
			InitializeRotation(state);
		}

		/// <summary>I DON'T KNOW IF THIS WORKS YETTTT!!</summary>
		public void ResetPlatform()
		{
			InitializeRotation(false);
			if (isOppositePlatformType)
			{
				TS.GetRotation(this.ID, out vec3 rot);
				rot.x = 0;
				TS.SetRotation(this.ID, rot);
			}
			else
			{
				TS.GetRotation(this.ID, out vec3 rot);
				rot.z = 0;
				TS.SetRotation(this.ID, rot);
			}
		}

		/// <summary>Assign variables based on the constants set in Start() and the given <c>state</c>.</summary>
		void InitializeRotation(bool state)
		{
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

		/// <summary>Send the "signal" that a rotation should take place.</summary>
		void StartRotation()
		{
			shouldRotate = true;
		}

		void PerformRotationZ()
		{
			TS.GetRotation(this.ID, out vec3 rot);
			float amountComplete = InverseLerp(rotateStartAngle, rotateEndAngle, rot.z);

			if (amountComplete < 1)
			{
				if (platformState == false)
					rot.z += goUpRotationSpeed * Time.deltaTime * rotationDirection;
				else
					rot.z += goDownRotationSpeed * Time.deltaTime * rotationDirection;

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
				if (platformState == false)
					rot.x += goUpRotationSpeed * Time.deltaTime * rotationDirection;
				else
					rot.x += goDownRotationSpeed * Time.deltaTime * rotationDirection;

				TS.SetRotation(this.ID, rot);
			}
			else // if (amountComplete >= 1)
			{
				shouldRotate = false;
				platformState = IsInRange(rot.x, platformActiveAngle);
			}
		}

		void TrembleZ()
		{
			const float amplitude = 0.125f;
			const float frequency = 60.0f;

			trembleTimer += Time.deltaTime;

			if (trembleTimer < trembleDuration)
			{
				TS.GetPosition(this.ID, out vec3 pos);
				pos.z = originalPosition.z + amplitude * MathF.Sin(globalTimer * frequency);
				TS.SetPosition(this.ID, pos);
			}
			else
			{
				// send the 'signal' to actualy begin rotation and stop trembling
				StartRotation();
				// transform.Position = originalPosition;
				trembleTimer = 0;
				shouldTremble = false;
				return;
			}
		}

		void TrembleX()
		{
			const float amplitude = 0.125f;
			const float frequency = 60.0f;

			trembleTimer += Time.deltaTime;

			if (trembleTimer < trembleDuration)
			{
				TS.GetPosition(this.ID, out vec3 pos);
				pos.x = originalPosition.x + amplitude * MathF.Sin(globalTimer * frequency);
				TS.SetPosition(this.ID, pos);
			}
			else
			{
				// send the 'signal' to actualy begin rotation and stop trembling
				StartRotation();
				// transform.Position = originalPosition;
				trembleTimer = 0;
				shouldTremble = false;
				return;
			}
		}

		#region Helper Functions
		/*
		void Swap(ref float a, ref float b)
		{
			// float temp = a;
			// a = b;
			// b = temp;
			(b, a) = (a, b); // <- ooh, that's new! :O
		}
		*/

		float InverseLerp(float start, float end, float output)
		{
			// r = p + (t)(q-p) <- Lerp formula
			// r - p = (t)(q-p)
			// t = (r - p) / (q - p)
			return (output - start) / (end - start);
		}

		bool IsInRange(float angle, float direction)
		{
			const float rangeInDegrees = 40.0f;
			if (direction - rangeInDegrees <= angle && angle <= direction + rangeInDegrees)
				return true;
			else
				return false;
		}

		/// <summary>sometimes (0, 0, 0) becomes (0, -0, 0) :_)</summary>
		bool HasNoRotation(vec3 rot)
		{
			return IsVec3Equal(rot, vec3.Zero);
		}

		/// <summary>preparing for the event that (0, 180, 0) becomes (0, -180, 0)</summary>
		bool HasFlippedRotation(vec3 rot)
		{
			return IsVec3Equal(rot, new vec3(0, 180, 0));
		}

		/// <summary>sometimes (0, 180, 0) becomes (-180, 0, -180) :_)</summary>
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
		#endregion
	}
}
