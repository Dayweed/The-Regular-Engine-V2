using GlmSharp;
using System.Collections.Generic;

namespace TRE
{
    public class CreditSceneLogic : Entity
    {
        Entity mCreditText;
        float m_ScrollSpeed = 100f;

        public void Start()
        {
            mCreditText = ECSManager.FindEntityByName("Credits");
        }

        public void Update()
        {
            vec3 currentpos = mCreditText.GetComponent<Transform>().Position;
            currentpos.y -= Time.GetDeltaTime() * m_ScrollSpeed;
            TransformSystem.SetPosition(mCreditText.ID, currentpos);
        }
    }
}
