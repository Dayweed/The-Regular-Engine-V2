using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;

namespace TRE
{
    public class VFX_Emerge : Entity
    {
        vec3 OriginalScale;
        vec3 OriginalRotation;
        vec3 OriginalPosition;

        vec3 StartScale;

        vec3 ScaleVec;

        // End is based on where the entity is at
        vec3 EndScale;

        vec3 RotateVec;

        vec3 PositionVec;

        bool emerging;
        bool shrinking;
        bool idle = true;

        float scaleSpeed = 80.0f;
        float rotateSpeed = 15.0f;
        float moveSpeed = 65.0f;

        vec3 MinScaleOffset;
        vec3 MaxScaleOffset;

        float coolDown = 0;
        float coolDownDefault = 0.45f;

        SpriteRenderer MyRenderer;

        public void Start()
        {
            MyRenderer = GetComponent<SpriteRenderer>();

            OriginalPosition = transform.Position;
            OriginalScale = transform.Scale;
            OriginalRotation = transform.Rotation;

            StartScale = new vec3(0, 0, 1);

            RotateVec = new vec3(0, 0, 15f);

            PositionVec = new vec3(-16, -9, 0);
        }

        public void Update()
        {
            if (idle) return;

            transform.Rotation += RotateVec * rotateSpeed * Time.deltaTime;

            if (!idle && (emerging || shrinking))
            {
                if (transform.Scale.x >= MinScaleOffset.x && transform.Scale.x <= MaxScaleOffset.x
                    && transform.Scale.y >= MinScaleOffset.y && transform.Scale.y <= MaxScaleOffset.y
                    && transform.Scale.z >= MinScaleOffset.z && transform.Scale.z <= MaxScaleOffset.z)
                {
                    transform.Scale = EndScale;
                    if (emerging)
                    {
                        coolDown = coolDownDefault;
                        emerging = false;
                    }

                    if (shrinking)
                    {
                        Reset();
                        idle = true;
                        shrinking = false;
                    }
                }
                else
                {
                    transform.Scale += ScaleVec * scaleSpeed * Time.deltaTime;

                    if (shrinking)
                    {
                        transform.Position += PositionVec * moveSpeed * Time.deltaTime;
                    }
                }
            }
            else if (!emerging && !shrinking && !idle && coolDown > 0)
            {
                coolDown -= Time.deltaTime;
            }
            else if (!emerging && !shrinking && !idle && coolDown <= 0)
            {
                coolDown = 0f;
                ShrinkBack();
            }
        }

        public void Emerge()
        {
            if (emerging) return;

            MyRenderer.isVisible = true;
            EndScale = transform.Scale;

            emerging = true;
            shrinking = false;
            idle = false;

            ScaleVec = (EndScale - StartScale).Normalized;

            transform.Scale = StartScale;

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
        }

        public void ShrinkBack()
        {
            if (shrinking) return;

            MyRenderer.isVisible = true;
            StartScale = transform.Scale;
            EndScale = new vec3(1,1,1);

            emerging = false;
            shrinking = true;
            idle = false;

            ScaleVec = (EndScale - StartScale).Normalized;

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
        }

        public void Reset()
        {
            transform.Position = OriginalPosition;
            transform.Scale = OriginalScale;
            transform.Rotation = OriginalRotation;

            MyRenderer.isVisible = false;
        }
    }
}