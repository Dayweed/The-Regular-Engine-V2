using GlmSharp;

namespace TRE
{
	using TS = TransformSystem;
	public class DioramaSpin : Entity
	{
		const float rotSpeed = 50.0f;

		public void Update()
		{
			TS.GetRotation(this.ID, out vec3 rot);
			rot.y += rotSpeed * Time.deltaTime;
			TS.SetRotation(this.ID, rot);
		}
	}
}
