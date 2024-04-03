using GlmSharp;
using System;

namespace TRE
{
	public class VFX_SlapOn : Entity
	{
		vec3 StartRotate;
		vec3 StartScale;
		vec4 StartColor;

		vec3 RotateVec;
		vec3 ScaleVec;
		vec4 ColorVec;

		vec3 EndRotation;
		vec3 EndScale;
		vec4 EndColor;

		bool slapping;
		bool doneRotate;
		bool doneScale;
		bool doneColor;

		const float rotationSpeed = 8.5f;
		const float scaleSpeed = 20.0f;
		const float colorSpeed = 5f;
		const float offsetMult = 0.05f;

		vec3 MinRotateOffset;
		vec3 MinScaleOffset;
		vec4 MinColorOffset;

		vec3 MaxRotateOffset;
		vec3 MaxScaleOffset;
		vec4 MaxColorOffset;

		float TimerToStop;
		const float TimerToStopDefault = 1f;

		/// <summary>This is to ensure it wont be stuck looping forever.</summary>
		const float OffsetScale = 1f;

		/// <summary>Only becomes true if was called and finish VFX.</summary>
		public bool CompleteVFX = false; // 

		public void Start()
		{
			if (Scene.GetSceneName() == "ResultScreen")
			{
				StartRotate = new vec3(0, 0, 0f);
				StartScale = new vec3(12f, 12f, 1);
				StartColor = new vec4(1, 1, 1, 0);
			}
			else
			{
				StartRotate = new vec3(0, 0, 20.0f);
				StartScale = new vec3(20, 20, 1);
				StartColor = new vec4(1, 1, 1, 0);
			}
		}

		public void Update()
		{
			if (slapping)
			{
				TimerToStop -= Time.deltaTime;
				if (TimerToStop <= 0)
				{
					doneRotate = true;
					doneScale = true;
					doneColor = true;
					slapping = false;
					transform.Rotation = EndRotation;
					transform.Scale = EndScale;
					GetComponent<SpriteRenderer>().Color = EndColor;
				}

				if (IsVec3WithinRange(MinRotateOffset, MaxRotateOffset, transform.Rotation))
				{
					doneRotate = true;
				}
				else
				{
					transform.Rotation = vec3.Lerp(transform.Rotation, EndRotation, rotationSpeed * Time.deltaTime);
				}

				if (IsVec3WithinRange(MinScaleOffset, MaxScaleOffset, transform.Scale))
				{
					doneScale = true;
				}
				else if (!doneScale)
				{
					transform.Scale = vec3.Lerp(transform.Scale, EndScale, scaleSpeed * Time.deltaTime);
				}

				if (IsVec4WithinRange(MinColorOffset, MaxColorOffset, GetComponent<SpriteRenderer>().Color))
				{
					doneColor = true;
				}
				else if (!doneColor)
				{
					SpriteRenderer spriteRenderer = GetComponent<SpriteRenderer>();
					spriteRenderer.Color = vec4.Lerp(spriteRenderer.Color, EndColor, colorSpeed * Time.deltaTime);
				}

				if (doneRotate && doneScale && doneColor)
				{
					CompleteVFX = true;
					slapping = false;
					transform.Rotation = EndRotation;
					transform.Scale = EndScale;
					GetComponent<SpriteRenderer>().Color = EndColor;
				}
			}
		}

		public bool CompletedVFX()
		{
			return CompleteVFX;
		}

		public void SlapOn()
		{
			if (slapping) return;

			CompleteVFX = false;

			// Force stop
			TimerToStop = TimerToStopDefault;

			EndRotation = transform.Rotation;
			EndScale = transform.Scale;
			EndColor = GetComponent<SpriteRenderer>().Color;

			slapping = true;
			doneRotate = false;
			doneScale = false;
			doneColor = false;

			RotateVec = (EndRotation - StartRotate);
			ScaleVec = (EndScale - StartScale);
			ColorVec = (EndColor - StartColor);

			if (RotateVec.Length > 0) RotateVec = RotateVec.Normalized;
			if (ScaleVec.Length > 0) ScaleVec = ScaleVec.Normalized;
			if (ColorVec.Length > 0) ColorVec = ColorVec.Normalized;


			transform.Rotation = StartRotate;
			transform.Scale = StartScale;
			GetComponent<SpriteRenderer>().Color = StartColor;

			vec3 rotOff = RotateVec * offsetMult * Time.deltaTime;
			MinRotateOffset = vec3.Min(EndRotation - rotOff * OffsetScale, EndRotation + rotOff * OffsetScale);
			MaxRotateOffset = vec3.Max(EndRotation - rotOff * OffsetScale, EndRotation + rotOff * OffsetScale);

			vec3 scaOff = ScaleVec * offsetMult * Time.deltaTime;

			MinScaleOffset = vec3.Min(EndScale - scaOff * OffsetScale, EndScale + scaOff * OffsetScale);
			MaxScaleOffset = vec3.Max(EndScale - scaOff * OffsetScale, EndScale + scaOff * OffsetScale);

			vec4 colOff = ColorVec * offsetMult * Time.deltaTime;
			MinColorOffset = vec4.Min(EndColor - colOff * OffsetScale, EndColor + colOff * OffsetScale);
			MaxColorOffset = vec4.Max(EndColor - colOff * OffsetScale, EndColor + colOff * OffsetScale);
		}

		bool IsVec3WithinRange(vec3 min, vec3 max, vec3 value)
		{
			bool isWithinRangeX = min.x <= value.x && value.x <= max.x;
			bool isWithinRangeY = min.y <= value.y && value.y <= max.y;
			bool isWithinRangeZ = min.z <= value.z && value.z <= max.z;
			return isWithinRangeX && isWithinRangeY && isWithinRangeZ;
		}

		bool IsVec4WithinRange(vec4 min, vec4 max, vec4 value)
		{
			bool isWithinRangeX = min.x <= value.x && value.x <= max.x;
			bool isWithinRangeY = min.y <= value.y && value.y <= max.y;
			bool isWithinRangeZ = min.z <= value.z && value.z <= max.z;
			bool isWithinRangeW = min.w <= value.w && value.w <= max.w;
			return isWithinRangeX && isWithinRangeY && isWithinRangeZ && isWithinRangeW;
		}
	}
}