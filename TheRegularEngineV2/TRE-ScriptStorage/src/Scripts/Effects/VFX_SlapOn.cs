using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;

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
		SpriteRenderer MyRenderer;

		float rotationSpeed = 8.5f;
		float scaleSpeed = 12.0f;
		float colorSpeed = 0.5f;

		vec3 MinRotateOffset;
		vec3 MinScaleOffset;
		vec4 MinColorOffset;

		vec3 MaxRotateOffset;
		vec3 MaxScaleOffset;
		vec4 MaxColorOffset;

		float TimerToStop;
		float TimerToStopDefault = 2f;

		float OffsetScale = 1f;			// This is to ensure it wont be stuck looping forever

		public bool CompleteVFX = false; // Only becomes true if was called and finish vfx

		public void Start()
		{
			MyRenderer = GetComponent<SpriteRenderer>();

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
					MyRenderer.Color = EndColor;
				}

				if (transform.Rotation.x >= MinRotateOffset.x && transform.Rotation.x <= MaxRotateOffset.x
					&& transform.Rotation.y >= MinRotateOffset.y && transform.Rotation.y <= MaxRotateOffset.y
					&& transform.Rotation.z >= MinRotateOffset.z && transform.Rotation.z <= MaxRotateOffset.z)
				{
					doneRotate = true;
				}
				else
				{
					//transform.Rotation += RotateVec * rotationSpeed * Time.deltaTime;
					float x = MathF.Lerp(transform.Rotation.x, EndRotation.x, rotationSpeed * Time.deltaTime);
                    float y = MathF.Lerp(transform.Rotation.y, EndRotation.y, rotationSpeed * Time.deltaTime);
                    float z = MathF.Lerp(transform.Rotation.z, EndRotation.z, rotationSpeed * Time.deltaTime);
					transform.Rotation = new vec3(x, y, z);
                }

				if (transform.Scale.x >= MinScaleOffset.x && transform.Scale.x <= MaxScaleOffset.x
					&& transform.Scale.y >= MinScaleOffset.y && transform.Scale.y <= MaxScaleOffset.y
					&& transform.Scale.z >= MinScaleOffset.z && transform.Scale.z <= MaxScaleOffset.z)
				{
					doneScale = true;
				}
				else if (!doneScale)
				{
					//transform.Scale += ScaleVec * scaleSpeed * Time.deltaTime;
                    float x = MathF.Lerp(transform.Scale.x, EndScale.x, scaleSpeed * Time.deltaTime);
                    float y = MathF.Lerp(transform.Scale.y, EndScale.y, scaleSpeed * Time.deltaTime);
                    float z = MathF.Lerp(transform.Scale.z, EndScale.z, scaleSpeed * Time.deltaTime);
                    transform.Scale = new vec3(x, y, z);
                }

				if (MyRenderer.Color.x >= MinColorOffset.x && MyRenderer.Color.x <= MaxColorOffset.x
					&& MyRenderer.Color.y >= MinColorOffset.y && MyRenderer.Color.y <= MaxColorOffset.y
					&& MyRenderer.Color.z >= MinColorOffset.z && MyRenderer.Color.z <= MaxColorOffset.z
					&& MyRenderer.Color.w >= MinColorOffset.w && MyRenderer.Color.w <= MaxColorOffset.w)
				{
					doneColor = true;
				}
				else if (!doneColor)
				{
					//MyRenderer.Color += ColorVec * colorSpeed * Time.deltaTime;
                    float x = MathF.Lerp(MyRenderer.Color.x, EndColor.x, colorSpeed * Time.deltaTime);
                    float y = MathF.Lerp(MyRenderer.Color.y, EndColor.y, colorSpeed * Time.deltaTime);
                    float z = MathF.Lerp(MyRenderer.Color.z, EndColor.z, colorSpeed * Time.deltaTime);
                    float w = MathF.Lerp(MyRenderer.Color.w, EndColor.w, colorSpeed * Time.deltaTime);
                    MyRenderer.Color = new vec4(x, y, z, w);
                }

				if (doneRotate && doneScale && doneColor)
				{
					CompleteVFX = true;
                    slapping = false;
					transform.Rotation = EndRotation;
					transform.Scale = EndScale;
					MyRenderer.Color = EndColor;
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
			EndColor = MyRenderer.Color;

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
			MyRenderer.Color = StartColor;


			vec3 rotOff = RotateVec * rotationSpeed * 0.5f * Time.deltaTime;
			MinRotateOffset = new vec3(
				(EndRotation.x - rotOff.x < EndRotation.x + rotOff.x) ? EndRotation.x - rotOff.x * OffsetScale : EndRotation.x + rotOff.x * OffsetScale,
				(EndRotation.y - rotOff.y < EndRotation.y + rotOff.y) ? EndRotation.y - rotOff.y * OffsetScale : EndRotation.y + rotOff.y * OffsetScale,
				(EndRotation.z - rotOff.z < EndRotation.z + rotOff.z) ? EndRotation.z - rotOff.z * OffsetScale : EndRotation.z + rotOff.z * OffsetScale
                );
			MaxRotateOffset = new vec3(
				(EndRotation.x - rotOff.x > EndRotation.x + rotOff.x) ? EndRotation.x - rotOff.x * OffsetScale : EndRotation.x + rotOff.x * OffsetScale,
				(EndRotation.y - rotOff.y > EndRotation.y + rotOff.y) ? EndRotation.y - rotOff.y * OffsetScale : EndRotation.y + rotOff.y * OffsetScale,
				(EndRotation.z - rotOff.z > EndRotation.z + rotOff.z) ? EndRotation.z - rotOff.z * OffsetScale : EndRotation.z + rotOff.z * OffsetScale
                );

			vec3 scaOff = ScaleVec * scaleSpeed * 0.5f * Time.deltaTime;
			MinScaleOffset = new vec3(
				(EndScale.x - scaOff.x < EndScale.x + scaOff.x) ? EndScale.x - scaOff.x * OffsetScale : EndScale.x + scaOff.x * OffsetScale,
				(EndScale.y - scaOff.y < EndScale.y + scaOff.y) ? EndScale.y - scaOff.y * OffsetScale : EndScale.y + scaOff.y * OffsetScale,
				(EndScale.z - scaOff.z < EndScale.z + scaOff.z) ? EndScale.z - scaOff.z * OffsetScale : EndScale.z + scaOff.z * OffsetScale
                );
			MaxScaleOffset = new vec3(
				(EndScale.x - scaOff.x > EndScale.x + scaOff.x) ? EndScale.x - scaOff.x * OffsetScale : EndScale.x + scaOff.x * OffsetScale,
				(EndScale.y - scaOff.y > EndScale.y + scaOff.y) ? EndScale.y - scaOff.y * OffsetScale : EndScale.y + scaOff.y * OffsetScale,
				(EndScale.z - scaOff.z > EndScale.z + scaOff.z) ? EndScale.z - scaOff.z * OffsetScale : EndScale.z + scaOff.z * OffsetScale
                );

			vec4 colOff = ColorVec * colorSpeed * 0.5f * Time.deltaTime;
			MinColorOffset = new vec4(
				(EndColor.x - colOff.x < EndColor.x + colOff.x) ? EndColor.x - colOff.x * OffsetScale : EndColor.x + colOff.x * OffsetScale,
				(EndColor.y - colOff.y < EndColor.y + colOff.y) ? EndColor.y - colOff.y * OffsetScale : EndColor.y + colOff.y * OffsetScale,
				(EndColor.z - colOff.z < EndColor.z + colOff.z) ? EndColor.z - colOff.z * OffsetScale : EndColor.z + colOff.z * OffsetScale,
				(EndColor.w - colOff.w < EndColor.w + colOff.w) ? EndColor.w - colOff.w * OffsetScale : EndColor.w + colOff.w * OffsetScale
                );
			MaxColorOffset = new vec4(
				(EndColor.x - colOff.x > EndColor.x + colOff.x) ? EndColor.x - colOff.x * OffsetScale : EndColor.x + colOff.x * OffsetScale,
				(EndColor.y - colOff.y > EndColor.y + colOff.y) ? EndColor.y - colOff.y * OffsetScale : EndColor.y + colOff.y * OffsetScale,
				(EndColor.z - colOff.z > EndColor.z + colOff.z) ? EndColor.z - colOff.z * OffsetScale : EndColor.z + colOff.z * OffsetScale,
				(EndColor.w - colOff.w > EndColor.w + colOff.w) ? EndColor.w - colOff.w * OffsetScale : EndColor.w + colOff.w * OffsetScale
                );
		}
	}
}