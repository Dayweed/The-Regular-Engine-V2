using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

namespace TRE
{
	using AS = AudioSystem;

	public class JumpPad : Entity
	{
		public bool isActivated = false;
		private string activatedMat = "JumpPadActivated.material";
		private string deactivatedMat = "JumpPad.material";

		private ulong jumppadSFX;
		private bool isPlaying = false;

		public JumpPad()
		{

		}

		public void Start()
		{
			if (name == "JumpPad_2")
			{
				isActivated = true;
			}
			GetComponent<MeshRenderer>().Material = isActivated ? activatedMat : deactivatedMat;

			jumppadSFX = ECSManager.FindIDFromName("SFX_JumpPad");
		}

		public void Update()
		{

		}

		public void ActivatePad(bool isActive)
		{
			// This is to make it actually activate cos for some reason it doesnt now :/
			isActivated = isActive;
			GetComponent<MeshRenderer>().Material = isActivated ? activatedMat : deactivatedMat;

			if(isActivated && !isPlaying)
			{
				isPlaying = true;
				AS.Play(jumppadSFX);
			}
			else if(!isActivated && isPlaying)
			{
				isPlaying = false;
			}
		}
	}
}