namespace TRE
{
	public class SkyboxSetter : Entity
	{

		public void Start()
		{
			string currentscene = Scene.GetSceneName();

            if (currentscene == "Level_1")
            {
				RendererSystem.Engine_SetSkyboxEnvironment("skybox1-desert.png", "skybox2-desert.png", "skybox3-desert.png", "skybox4-desert.png", "skybox5-desert.png", "skybox6-desert.png");
            }

			if (currentscene == "Level_2")
			{
                RendererSystem.Engine_SetSkyboxEnvironment("skybox1-l2.png", "skybox2-l2.png", "skybox3-l2.png", "skybox4-l2.png", "skybox5-l2.png", "skybox6-l2.png");
            }
		}
	}
}
