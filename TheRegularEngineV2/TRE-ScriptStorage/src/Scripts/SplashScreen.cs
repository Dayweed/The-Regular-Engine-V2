using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TRE
{
	public class SplashScreen : Entity
	{
		SpriteRenderer spriteRenderer;
		private string[] splashArray = { "scene_digipen.png", "scene_fmod.png" };
		private int splashIndex = 0;

		public void Start()
		{
			spriteRenderer = GetComponent<SpriteRenderer>();

			spriteRenderer.Texture = splashArray[splashIndex];

            ScenePostEffectsSystem.Engine_ShrinkVignette(2);
        }

		public void Update()
        {
			if (ScenePostEffectsSystem.VignetteState == ScenePostEffectsSystem.STATE.IN)
			{
				++splashIndex;
				if (splashIndex < splashArray.Length)
                {
                    spriteRenderer.Texture = splashArray[splashIndex];
                }
            }

            if (ScenePostEffectsSystem.VignetteState == ScenePostEffectsSystem.STATE.OUT)
            {
				if (splashIndex < splashArray.Length - 1)
                {
                    ScenePostEffectsSystem.Engine_ShrinkVignette(2);
				}
				else
                {
                    Scene.TransitionScene("Start", 2.0f);
                }
            }
        }
	}
}
