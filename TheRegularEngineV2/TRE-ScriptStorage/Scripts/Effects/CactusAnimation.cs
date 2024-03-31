using GlmSharp;

namespace TRE
{
	using TS = TransformSystem;

	class CactusAnimation : Entity
	{
		private Transform cactusXform;
		private vec3 initialScale;
		private vec3 currentScale;
		private vec3 currentRotation;
		private const float bigScale = 1.25f;
		private const float smallScale = 1;
		private const float lerpSpeed = 5f;
		private bool isBig = false;
		private bool isSmall = true;
		private bool isLeft;
		// private bool isRight;
		private const float left = 30f;
		private const float right = -30f;

		public void Start()
		{
			//get the transform component of the cactus
			cactusXform = GetComponent<Transform>();
			initialScale = cactusXform.Scale;
			currentScale = initialScale;
			currentRotation = cactusXform.Rotation;
			if (this.ID % 3 == 0)
			{
				isLeft = true;
				// isRight = false;
			}
			else
			{
				isLeft = false;
				// isRight = true;
			}
		}

		public void Update()
		{
			if (isSmall)
			{
				currentScale.y = MathF.Lerp(currentScale.y, bigScale, lerpSpeed * Time.deltaTime);
				currentRotation.z = MathF.Lerp(currentRotation.z, isLeft ? left : right, lerpSpeed * Time.deltaTime);
				TS.SetScaling(this.ID, currentScale);
				TS.SetRotation(this.ID, currentRotation);
				if (currentScale.y >= bigScale - 0.01f)
				{
					isSmall = false;
					isBig = true;
					isLeft = !isLeft;
				}
			}
			else if (isBig)
			{
				currentScale.y = MathF.Lerp(currentScale.y, smallScale, lerpSpeed * Time.deltaTime);
				currentRotation.z = MathF.Lerp(currentRotation.z, isLeft ? left : right, lerpSpeed * Time.deltaTime);
				TS.SetScaling(this.ID, currentScale);
				TS.SetRotation(this.ID, currentRotation);
				if (currentScale.y <= smallScale + 0.01f)
				{
					isBig = false;
					isSmall = true;
					isLeft = !isLeft;
				}
			}
		}
	}
}
