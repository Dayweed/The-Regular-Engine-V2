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

        bool MoleyInside = false;
        bool HoleyInside = false;

        public void Start()
		{
            Moley = ECSManager.FindEntityByName("Moley");
            Holey = ECSManager.FindEntityByName("Holey");
        }

		public void Update()
		{
            if (MoleyInside && InputSystem.GetKeyHold(InputKeys.Space) && !MoleyApprove)
            {
                MoleyApprove = true;
            }
            if (HoleyInside && InputSystem.GetKeyHold(InputKeys.Enter) && !HoleyApprove)
            {
                HoleyApprove = true;
            }
        }

        private void OnTriggerEnter(System.UInt64 otherID)
        {
            // Check if Moley or Holey is inside
            if (otherID == Moley.ID)
            {
                MoleyInside = true;
            }
            if (otherID == Holey.ID)
            {
                HoleyInside = true;
            }
        }

        private void OnTriggerExit(System.UInt64 otherID)
        {
            // Check if Moley or Holey left
            if (otherID == Moley.ID)
            {
                MoleyInside = false;
                MoleyApprove = false;
            }
            if (otherID == Holey.ID)
            {
                HoleyInside = false;
                HoleyApprove = false;
            }
        }

        public bool MolesApproved()
        {
            return MoleyApprove && HoleyApprove;
        }

        public bool MolesInside()
        {
            return MoleyInside && HoleyInside;
        }

        public void ResetMoles()
        {
            MoleyInside = false;
            HoleyInside = false;
            MoleyApprove = false;
            HoleyApprove = false;
        }
    }
}