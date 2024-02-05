using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

namespace TRE
{
	public class JumpPadSwitch : Entity
	{
		public Entity JumpPad;
		public Entity JumpPad_1;

		public Entity UpButton;
		public Entity UpButton_1;

		public JumpPadSwitch()
		{

		}

		public void OnCreate()
		{
			UpButton = ECSManager.FindEntityByName("UpButton");
			UpButton_1 = ECSManager.FindEntityByName("UpButton_1");

			JumpPad = ECSManager.FindEntityByName("JumpPad");
			JumpPad_1 = ECSManager.FindEntityByName("JumpPad_1");
			if (JumpPad == null)
			{
				Debug.LogError("JumpPadSwitch JumpPad is null!");
				return;
			}
			if (JumpPad.GetComponent<JumpPad>() == null)
			{
				Debug.LogError("JumpPadSwitch JumpPad Component is null!");
				return;
			}
		}

		public void Update()
		{

		}

		public void OnTriggerStay(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
			// Check is activated jumppad
			if (other.CompareTag("Red") || other.CompareTag("Blue"))
			{
				JumpPad.GetComponent<JumpPad>().ActivatePad(true);
				JumpPad_1.GetComponent<JumpPad>().ActivatePad(true);
				UpButton.SetActive(false);
				UpButton_1.SetActive(false);
			}
		}

		public void OnTriggerExit(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
			// Check is activated jumppad
			if (other.CompareTag("Red") || other.CompareTag("Blue"))
			{
				JumpPad.GetComponent<JumpPad>().ActivatePad(false);
				JumpPad_1.GetComponent<JumpPad>().ActivatePad(false);
				UpButton.SetActive(true);
				UpButton_1.SetActive(true);
			}
		}
	}
}