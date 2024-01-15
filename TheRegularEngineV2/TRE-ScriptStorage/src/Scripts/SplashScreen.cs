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

		public void Start()
		{
			spriteRenderer = GetComponent<SpriteRenderer>();

			Debug.Log(spriteRenderer.Texture);

            ScenePostEffectsSystem.Engine_ShrinkVignette(10);
        }

		public void Update()
        {
			if (ScenePostEffectsSystem.VignetteState == ScenePostEffectsSystem.STATE.IN)
			{
				Debug.Log("INNN!!!!");
			}
			if (ScenePostEffectsSystem.VignetteState == ScenePostEffectsSystem.STATE.OUT)
			{
				Debug.Log("OUTTT!!!!");
			}
        }
	}
}
