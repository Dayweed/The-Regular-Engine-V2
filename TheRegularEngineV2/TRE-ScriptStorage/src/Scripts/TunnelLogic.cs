using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;

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

		// Signposts
		Entity SignpostFeature;
		Entity SignpostMoley;
		Entity SignpostHoley;

		float SignpostGoalMoleyPosY = 0;
		float SignpostGoalHoleyPosY = 0;
		float SignpostHiddenPosY = -28;
		float SignpostDisplayPosY = -14;
		float SignpostMoveSpeed = 5f;
		float SignpostOffset = 0.05f;

		public void Start()
		{
			Moley = ECSManager.FindEntityByName("Moley");
			Holey = ECSManager.FindEntityByName("Holey");

			SignpostFeature = parenting.GetChildFromName("SignpostFeature");
			SignpostMoley = parenting.GetChildFromName("SignpostMoleyApprove");
			SignpostHoley = parenting.GetChildFromName("SignpostHoleyApprove");

			if (SignpostFeature != null && SignpostMoley != null && SignpostHoley != null)
			{
				SignpostFeature.GetComponent<VFX_SignPostBounce>().Pause();
				SignpostMoley.GetComponent<VFX_SignPostBounce>().Pause();
				SignpostHoley.GetComponent<VFX_SignPostBounce>().Pause();

				SignpostGoalMoleyPosY = SignpostHiddenPosY;
				SignpostGoalHoleyPosY = SignpostHiddenPosY;
			}
		}

		public void Update()
		{
			if (MoleyInside && InputSystem.GetKeyHold(InputKeys.Space) && Moley.GetComponent<MoleyController>().isControllable && !MoleyApprove)
			{
				MoleyApprove = true;
			}
			if (HoleyInside && InputSystem.GetKeyHold(InputKeys.Enter) && Moley.GetComponent<MoleyController>().isControllable && !HoleyApprove)
			{
				HoleyApprove = true;
			}

			// Determine signposts bounce
			if (SignpostFeature != null && SignpostMoley != null && SignpostHoley != null)
			{
				if (MoleyInside || HoleyInside)
				{
					SignpostFeature.GetComponent<VFX_SignPostBounce>().Resume();
					SignpostGoalMoleyPosY = MoleyInside ? SignpostDisplayPosY : SignpostHiddenPosY;
					SignpostGoalHoleyPosY = HoleyInside ? SignpostDisplayPosY : SignpostHiddenPosY;
				}
				else
				{
					SignpostFeature.GetComponent<VFX_SignPostBounce>().Pause();
					SignpostGoalMoleyPosY = SignpostHiddenPosY;
					SignpostGoalHoleyPosY = SignpostHiddenPosY;
				}

				// Move signpost to position
				vec3 SignpostMoleyPos = SignpostMoley.GetComponent<Transform>().Position;
				vec3 SignpostHoleyPos = SignpostHoley.GetComponent<Transform>().Position;
				if (Math.Abs(SignpostMoleyPos.y - SignpostGoalMoleyPosY) > SignpostOffset && SignpostMoley.GetComponent<VFX_SignPostBounce>().IsPaused())
				{
					float SignpostMoleyPosY = MathF.Lerp(SignpostMoleyPos.y, SignpostGoalMoleyPosY, SignpostMoveSpeed * Time.deltaTime);
					SignpostMoley.GetComponent<Transform>().Position = new vec3(SignpostMoleyPos.x, SignpostMoleyPosY, SignpostMoleyPos.z);
				}
				else if (SignpostMoley.GetComponent<VFX_SignPostBounce>().IsPaused())
				{
					SignpostMoley.GetComponent<Transform>().Position = new vec3(SignpostMoleyPos.x, SignpostGoalMoleyPosY, SignpostMoleyPos.z);
				}
				if (Math.Abs(SignpostHoleyPos.y - SignpostGoalHoleyPosY) > SignpostOffset && SignpostHoley.GetComponent<VFX_SignPostBounce>().IsPaused())
				{
					float SignpostHoleyPosY = MathF.Lerp(SignpostHoleyPos.y, SignpostGoalHoleyPosY, SignpostMoveSpeed * Time.deltaTime);
					SignpostHoley.GetComponent<Transform>().Position = new vec3(SignpostHoleyPos.x, SignpostHoleyPosY, SignpostHoleyPos.z);
				}
				else if (SignpostHoley.GetComponent<VFX_SignPostBounce>().IsPaused())
				{
					SignpostHoley.GetComponent<Transform>().Position = new vec3(SignpostHoleyPos.x, SignpostGoalHoleyPosY, SignpostHoleyPos.z);
				}

				if (MoleyApprove && Math.Abs(SignpostMoleyPos.y - SignpostDisplayPosY) <= SignpostOffset)
				{
					SignpostMoley.GetComponent<VFX_SignPostBounce>().Resume();
				}
				else if (!MoleyApprove)
				{
					SignpostMoley.GetComponent<VFX_SignPostBounce>().Pause();
				}
				if (HoleyApprove && Math.Abs(SignpostHoleyPos.y - SignpostDisplayPosY) <= SignpostOffset)
				{
					SignpostHoley.GetComponent<VFX_SignPostBounce>().Resume();
				}
				else if (!HoleyApprove)
				{
					SignpostHoley.GetComponent<VFX_SignPostBounce>().Pause();
				}
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