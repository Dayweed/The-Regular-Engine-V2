using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TRE
{
	public class Test : Entity
	{
		public float timer = 0;

		public void Start()
		{
			timer = 0;
		}

		public void Update()
		{
			// this block tests setting and getting IsActive for colliders
			if (HasComponent<BoxCollider>())
			{
				timer += Time.deltaTime;

				if (timer > 0.5f)
				{
					var comp = GetComponent<BoxCollider>();
					comp.IsActive = !comp.IsActive;
					timer = 0;
				}
			}
		}
	}
}
