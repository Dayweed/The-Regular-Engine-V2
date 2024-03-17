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

		public void Start()
		{
			if (name == "JumpPad_2")
			{
				isActivated = true;
			}
			GetComponent<MeshRenderer>().Material = isActivated ? activatedMat : deactivatedMat;

			jumppadSFX = ECSManager.FindIDFromName("SFX_JumpPad");
		}

		public void ActivatePad(bool isActive)
		{
			// This is to make it actually activate cos for some reason it doesnt now :/
			isActivated = isActive;
			GetComponent<MeshRenderer>().Material = isActivated ? activatedMat : deactivatedMat;

			if (isActivated && !isPlaying)
			{
				isPlaying = true;
				AS.Play(jumppadSFX);
			}
			else if (!isActivated && isPlaying)
			{
				isPlaying = false;
			}
		}
	}
}