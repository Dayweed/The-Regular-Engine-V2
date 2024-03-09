namespace TRE
{
	public class SkyboxSetter : Entity
	{

		public void Start()
		{
			RendererSystem.Engine_SetSkyboxEnvironment("skybox1-desert.png", "skybox2-desert.png", "skybox3-desert.png", "skybox4-desert.png", "skybox5-desert.png", "skybox6-desert.png");
		}
	}
}
