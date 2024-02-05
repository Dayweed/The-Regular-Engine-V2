using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;

namespace TRE
{
	public class VFX_FadeIn : Entity
    {
        vec4 OriginalColor;

		bool fading = false;
        bool doneFading = false;

		float fadingSpeed = 0.5f;

		SpriteRenderer MyRenderer;

		public void Start()
		{

        }

		public void Update()
		{
			if (!fading) return;

			if (HasComponent<SpriteRenderer>())
			{
				//float w = MathF.Lerp(MyRenderer.Color.w, OriginalColor.w, fadingSpeed * Time.deltaTime);
				float w = MyRenderer.Color.w + fadingSpeed * Time.deltaTime;
				MyRenderer.Color = new vec4(OriginalColor.x, OriginalColor.y, OriginalColor.z, w);

                // Make sure all its children also fade
                UpdateChildren(this, w);

                if (MyRenderer.Color.w >= 1)
				{
					ForceComplete();
				}
			}
		}

		private void UpdateChildren(Entity entity, float alpha)
		{
			for (int i = 0; i < entity.parenting.GetTotalChildren(); ++i)
			{
				Entity child = entity.parenting.GetChild(i);
				if (HasComponent<SpriteRenderer>())
				{
					SpriteRenderer spriteRenderer = child.GetComponent<SpriteRenderer>();
					spriteRenderer.Color = new vec4(spriteRenderer.Color.x, spriteRenderer.Color.y, spriteRenderer.Color.z, alpha);
				}
				UpdateChildren(child, alpha);
            }
		}

		public void FadeIn()
        {
            if (fading) return;

            MyRenderer = GetComponent<SpriteRenderer>();
            OriginalColor = MyRenderer.Color;

            MyRenderer.Color = new vec4(OriginalColor.x, OriginalColor.y, OriginalColor.z, 0);
			UpdateChildren(this, 0);

            MyRenderer.isVisible = true;
			fading = true;
			doneFading = false;
        }

		public bool DoneFading()
		{
			return doneFading;
		}

		public void ForceComplete()
		{
            MyRenderer.Color = new vec4(OriginalColor.x, OriginalColor.y, OriginalColor.z, 1);
            UpdateChildren(this, 1);
            fading = false;
            doneFading = true;
        }

		public void ForceStop()
		{
            fading = false;
        }
	}
}