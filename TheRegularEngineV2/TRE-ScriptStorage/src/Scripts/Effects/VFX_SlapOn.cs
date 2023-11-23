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

        float rotationSpeed = 17.5f;
        float scaleSpeed = 25.0f;
        float colorSpeed = 1f;

        vec3 MinRotateOffset;
        vec3 MinScaleOffset;
        vec4 MinColorOffset;

        vec3 MaxRotateOffset;
        vec3 MaxScaleOffset;
        vec4 MaxColorOffset;

        float TimerToStop;
        float TimerToStopDefault = 5f;

        public void Start()
        {
            MyRenderer = GetComponent<SpriteRenderer>();

            StartRotate = new vec3(0, 0, 20.0f);
            StartScale = new vec3(20, 20, 1);
            StartColor = new vec4(1, 1, 1, 0);
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
                }

                if (transform.Rotation.x >= MinRotateOffset.x && transform.Rotation.x <= MaxRotateOffset.x
                    && transform.Rotation.y >= MinRotateOffset.y && transform.Rotation.y <= MaxRotateOffset.y
                    && transform.Rotation.z >= MinRotateOffset.z && transform.Rotation.z <= MaxRotateOffset.z)
                {
                    doneRotate = true;
                }
                else
                {
                    transform.Rotation += RotateVec * rotationSpeed * Time.deltaTime;
                }

                if (transform.Scale.x >= MinScaleOffset.x && transform.Scale.x <= MaxScaleOffset.x
                    && transform.Scale.y >= MinScaleOffset.y && transform.Scale.y <= MaxScaleOffset.y
                    && transform.Scale.z >= MinScaleOffset.z && transform.Scale.z <= MaxScaleOffset.z)
                {
                    doneScale = true;
                }
                else if (!doneScale)
                {
                    transform.Scale += ScaleVec * scaleSpeed * Time.deltaTime;
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
                    MyRenderer.Color += ColorVec * colorSpeed * Time.deltaTime;
                }

                if (doneRotate && doneScale && doneColor)
                {
                    slapping = false;
                    transform.Rotation = EndRotation;
                    transform.Scale = EndScale;
                    MyRenderer.Color = EndColor;
                }
            }
        }

        public void SlapOn()
        {
            if (slapping) return;

            // Force stop
            TimerToStop = TimerToStopDefault;

            EndRotation = transform.Rotation;
            EndScale = transform.Scale;
            EndColor = MyRenderer.Color;

            slapping = true;
            doneRotate = false;
            doneScale = false;
            doneColor = false;

            RotateVec = (EndRotation - StartRotate).Normalized;
            ScaleVec = (EndScale - StartScale).Normalized;
            ColorVec = (EndColor - StartColor).Normalized;

            transform.Rotation = StartRotate;
            transform.Scale = StartScale;
            MyRenderer.Color = StartColor;


            vec3 rotOff = RotateVec * rotationSpeed * 0.5f * Time.deltaTime;
            MinRotateOffset = new vec3(
                (EndRotation.x - rotOff.x < EndRotation.x + rotOff.x) ? EndRotation.x - rotOff.x : EndRotation.x + rotOff.x,
                (EndRotation.y - rotOff.y < EndRotation.y + rotOff.y) ? EndRotation.y - rotOff.y : EndRotation.y + rotOff.y,
                (EndRotation.z - rotOff.z < EndRotation.z + rotOff.z) ? EndRotation.z - rotOff.z : EndRotation.z + rotOff.z
                );
            MaxRotateOffset = new vec3(
                (EndRotation.x - rotOff.x > EndRotation.x + rotOff.x) ? EndRotation.x - rotOff.x : EndRotation.x + rotOff.x,
                (EndRotation.y - rotOff.y > EndRotation.y + rotOff.y) ? EndRotation.y - rotOff.y : EndRotation.y + rotOff.y,
                (EndRotation.z - rotOff.z > EndRotation.z + rotOff.z) ? EndRotation.z - rotOff.z : EndRotation.z + rotOff.z
                );

            vec3 scaOff = ScaleVec * scaleSpeed * 0.5f * Time.deltaTime;
            MinScaleOffset = new vec3(
                (EndScale.x - scaOff.x < EndScale.x + scaOff.x) ? EndScale.x - scaOff.x : EndScale.x + scaOff.x,
                (EndScale.y - scaOff.y < EndScale.y + scaOff.y) ? EndScale.y - scaOff.y : EndScale.y + scaOff.y,
                (EndScale.z - scaOff.z < EndScale.z + scaOff.z) ? EndScale.z - scaOff.z : EndScale.z + scaOff.z
                );
            MaxScaleOffset = new vec3(
                (EndScale.x - scaOff.x > EndScale.x + scaOff.x) ? EndScale.x - scaOff.x : EndScale.x + scaOff.x,
                (EndScale.y - scaOff.y > EndScale.y + scaOff.y) ? EndScale.y - scaOff.y : EndScale.y + scaOff.y,
                (EndScale.z - scaOff.z > EndScale.z + scaOff.z) ? EndScale.z - scaOff.z : EndScale.z + scaOff.z
                );

            vec4 colOff = ColorVec * colorSpeed * 0.5f * Time.deltaTime;
            MinColorOffset = new vec4(
                (EndColor.x - colOff.x < EndColor.x + colOff.x) ? EndColor.x - colOff.x : EndColor.x + colOff.x,
                (EndColor.y - colOff.y < EndColor.y + colOff.y) ? EndColor.y - colOff.y : EndColor.y + colOff.y,
                (EndColor.z - colOff.z < EndColor.z + colOff.z) ? EndColor.z - colOff.z : EndColor.z + colOff.z,
                (EndColor.w - colOff.w < EndColor.w + colOff.w) ? EndColor.w - colOff.w : EndColor.w + colOff.w
                );
            MaxColorOffset = new vec4(
                (EndColor.x - colOff.x > EndColor.x + colOff.x) ? EndColor.x - colOff.x : EndColor.x + colOff.x,
                (EndColor.y - colOff.y > EndColor.y + colOff.y) ? EndColor.y - colOff.y : EndColor.y + colOff.y,
                (EndColor.z - colOff.z > EndColor.z + colOff.z) ? EndColor.z - colOff.z : EndColor.z + colOff.z,
                (EndColor.w - colOff.w > EndColor.w + colOff.w) ? EndColor.w - colOff.w : EndColor.w + colOff.w
                );
        }
    }
}