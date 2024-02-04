using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

namespace TRE
{
	public class TunnelLogic : Entity
	{
		Entity Moley;
		Entity Holey;
        bool MoleyApprove = false;
        bool HoleyApprove = false;

        public void Start()
		{
            Moley = ECSManager.FindEntityByName("Moley");
            Holey = ECSManager.FindEntityByName("Holey");
        }

		public void Update()
		{
			
        }

        private void OnTriggerStay(System.UInt64 otherID)
        {
            // Check if Moley or Holey is insdie
            if (otherID == Moley.ID && Moley.GetComponent<MoleyController>().isJumping)
            {
                MoleyApprove = true;
            }
            if (otherID == Holey.ID && Holey.GetComponent<HoleyController>().isJumping)
            {
                HoleyApprove = true;
            }
        }

        private void OnTriggerExit(System.UInt64 otherID)
        {
            // Check if Moley or Holey left
            if (otherID == Moley.ID)
            {
                MoleyApprove = false;
            }
            if (otherID == Holey.ID)
            {
                HoleyApprove = false;
            }
        }

        public bool MolesApproved()
        {
            return MoleyApprove && HoleyApprove;
        }
    }
}