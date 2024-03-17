using GlmSharp;

namespace TRE
{
	public class CreditSceneLogic : Entity
	{
		Entity mCreditText;
		const float m_ScrollSpeed = 500f;
		bool m_EndOfCredits = false;
		const float m_Endposition = -2850f;

		public void Start()
		{
			mCreditText = ECSManager.FindEntityByName("Credits");
			m_EndOfCredits = false;
		}

		public void Update()
		{
			ScrollingCredits();
		}

		public void ScrollingCredits()
		{
			vec3 currentpos = mCreditText.GetComponent<Transform>().Position;
			if (currentpos.y > m_Endposition)
			{
				currentpos.y -= Time.GetDeltaTime() * m_ScrollSpeed;
				TransformSystem.SetPosition(mCreditText.ID, currentpos);
			}
			else
			{
				m_EndOfCredits = true;
				//Debug.Log("Ended");
			}

			if (m_EndOfCredits)
			{
				PersistentSystem.SetValue("PrevScene", Scene.GetSceneName());
				Scene.TransitionScene("MainMenu", 5f);
			}
		}
	}
}
