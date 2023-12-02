using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

namespace TRE
{
	public class JumpPad : Entity
	{
		public bool isActivated = false;
		private string activatedMat = "Red_Moley.material";
		private string deactivatedMat = "HITW_Center.material";

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
		}

		public void Update()
		{

		}

		public void ActivatePad(bool isActive)
		{
			// This is to make it actually activate cos for some reason it doesnt now :/
			isActivated = isActive;
			GetComponent<MeshRenderer>().Material = isActivated ? activatedMat : deactivatedMat;
		}
	}
}