using GlmSharp;

namespace TRE
{
    public class CreditSceneLogic : Entity
    {
        Entity mCreditText;
        float m_ScrollSpeed = 100f;

        Entity mHoley;
        Entity mMoley;
        public vec3 JumpVelocity;

        public void Start()
        {
            mCreditText = ECSManager.FindEntityByName("Credits");
            mHoley = ECSManager.FindEntityByName("Holey");
            mMoley = ECSManager.FindEntityByName("Moley");
            JumpVelocity = new vec3(0, 70, 0);
        }

        public void Update()
        {
            ScrollingCredits();

            if (InputSystem.GetKeyTriggered(InputKeys.S))
            {
                PhysicsSystem.AddForce(mHoley.ID, JumpVelocity, ForceMode.VelocityChange);
                PhysicsSystem.AddForce(mMoley.ID, JumpVelocity, ForceMode.VelocityChange);
            }
        }
        public void ScrollingCredits()
        {
            vec3 currentpos = mCreditText.GetComponent<Transform>().Position;
            currentpos.y -= Time.GetDeltaTime() * m_ScrollSpeed;
            TransformSystem.SetPosition(mCreditText.ID, currentpos);
        }
    }
}
