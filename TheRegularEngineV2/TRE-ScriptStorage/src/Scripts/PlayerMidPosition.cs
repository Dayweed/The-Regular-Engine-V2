using System;
using System.Reflection;
using System.Runtime.CompilerServices;

using GlmSharp;

namespace TRE
{
	public class PlayerMidPosition : Entity
	{
		private Entity Player1;
		private Entity Player2;

		private Transform Player1Transform;
		private Transform Player2Transform;

		private vec3 thisPos;

		public float expectedYPos;

        private float lerpSpeed = 0.001f;

        public void Start()
		{
			Player1 = ECSManager.FindEntityByName("Moley");
			Player2 = ECSManager.FindEntityByName("Holey");

			Player1Transform = Player1.GetComponent<Transform>();
			Player2Transform = Player2.GetComponent<Transform>();
		}

		public void Update()
		{
            /*thisPos = Player1Transform.Position + Player2Transform.Position;
            thisPos /= 2;*/
			thisPos.y = MathF.Lerp(thisPos.y, expectedYPos, lerpSpeed);

			thisPos = new vec3((Player1Transform.Position.x + Player2Transform.Position.x) / 2,
								thisPos.y,
								(Player1Transform.Position.z + Player2Transform.Position.z) / 2);

            transform.Position = thisPos;
        }
	}
}
