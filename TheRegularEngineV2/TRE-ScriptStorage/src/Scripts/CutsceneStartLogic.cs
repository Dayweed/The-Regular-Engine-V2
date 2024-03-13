using System.Collections.Generic;

namespace TRE
{
	using AS = AudioSystem;

	public class CutsceneStartLogic : Entity
	{
		Entity Frame_1;
		Entity Frame_2;
		Entity Frame_3;
		Entity Frame_4;
		Entity Frame_5;
		Entity Frame_6;
		Entity Frame_7;
		Entity Frame_8;
		Entity Frame_9;
		Entity Frame_10;
		Entity SpaceToContinue;
		Entity SpaceToContinueBlack;
		Entity InvitationDialogue1;
		Entity InvitationDialogue2;
		Entity InvitationDialogue3;
		Entity InvitationDialogue4;
		Entity InvitationDialogue5;

        float currentTime = 0;
		float delayFrame = 5f;
		float delayScene = 7f;

		int currentFrame = 0;
		int pressSpaceCounter = 0;

		bool endCutscene = false;
		bool pressedSpaceTwice = false;

		private ulong birdSFX;
		private ulong dialogueSFX;
		private ulong invitationSFX;
		private ulong endBGM;
		private ulong BGM;
		private bool invitationSFXPlayed = false;

		List<Entity> frames = new List<Entity>();
		List<string> nextScenes = new List<string>();
		List<string> forcedScenes = new List<string>(); // This requires the user to press space manually to go to the next scene


		public void Start()
		{
			Frame_1 = ECSManager.FindEntityByName("Frame1");
			Frame_2 = ECSManager.FindEntityByName("Frame2");
			Frame_3 = ECSManager.FindEntityByName("Frame3");
			Frame_4 = ECSManager.FindEntityByName("Frame4");
			Frame_5 = ECSManager.FindEntityByName("Frame5");
			Frame_6 = ECSManager.FindEntityByName("Frame6");
			Frame_7 = ECSManager.FindEntityByName("Frame7");
			Frame_8 = ECSManager.FindEntityByName("Frame8");
			Frame_9 = ECSManager.FindEntityByName("Frame9");
			Frame_10 = ECSManager.FindEntityByName("Frame10");
			SpaceToContinue = ECSManager.FindEntityByName("SpaceToContinue");
			SpaceToContinueBlack = ECSManager.FindEntityByName("SpaceToContinueBlack");
			InvitationDialogue1 = ECSManager.FindEntityByName("InvitationDialogue1");
			InvitationDialogue2 = ECSManager.FindEntityByName("InvitationDialogue2");
			InvitationDialogue3 = ECSManager.FindEntityByName("InvitationDialogue3");
			InvitationDialogue4 = ECSManager.FindEntityByName("InvitationDialogue4");
			InvitationDialogue5 = ECSManager.FindEntityByName("InvitationDialogue5");

            Frame_1.SetActive(false);
			Frame_2.SetActive(false);
			Frame_3.SetActive(false);
			Frame_4.SetActive(false);
			Frame_5.SetActive(false);
			Frame_6.SetActive(false);
			Frame_7.SetActive(false);
			Frame_8.SetActive(false);
			Frame_9.SetActive(false);
			Frame_10.SetActive(false);
			SpaceToContinue.SetActive(false);
			SpaceToContinueBlack.SetActive(false);

			frames = new List<Entity>() { Frame_1, Frame_2, Frame_3, Frame_4, Frame_5, Frame_6, Frame_7, Frame_8, Frame_9, Frame_10 };
			nextScenes = new List<string>() { "Frame3", "Frame4", "Frame5", "Frame6", "Frame7", "Frame8", "Frame9" };
			forcedScenes = new List<string>() { "Frame4", "Frame5", "Frame6", "Frame7", "Frame8" };

			birdSFX = ECSManager.FindIDFromName("SFX_Bird");
			dialogueSFX = ECSManager.FindIDFromName("SFX_DIalogue");
			invitationSFX = ECSManager.FindIDFromName("SFX_Invitation");
			endBGM = ECSManager.FindIDFromName("BGM_EndLoop");
			BGM = ECSManager.FindIDFromName("BGM");

			currentFrame = 0;
			frames[currentFrame].SetActive(true);
			frames[currentFrame].GetComponent<VFX_FadeIn>().FadeIn();
			currentTime = delayFrame;
		}

		public void Update()
		{
			bool pressedSpace = InputSystem.GetKeyPress(InputKeys.Space);
			bool pressA = InputSystem.GetControllerButtonTriggered(0, InputSystem.Button.A);

            // Go to next scene
            if ((pressedSpace || pressA) && SpaceToContinue.GetActive() && currentFrame == frames.Count - 1)
			{
				if (ECSManager.IsValidEntity(BGM))
					AS.Stop(BGM);

				if (ECSManager.IsValidEntity(endBGM))
					AS.Play(endBGM);

				Scene.TransitionScene("Tutorial", delayScene);
				endCutscene = true;
			}

			if (currentFrame == 2)
			{
				if (ECSManager.IsValidEntity(birdSFX))
					AS.Stop(birdSFX);
			}

			if (currentFrame == 2 && !invitationSFXPlayed)
			{
				if (ECSManager.IsValidEntity(invitationSFX))
					AS.Play(invitationSFX);
				invitationSFXPlayed = true;
			}

			//letter frames: 3, 4, 5, 6, 7
			if (currentFrame >= 3 && currentFrame <= 7)
			{
				if (ECSManager.IsValidEntity(dialogueSFX))
					AS.Play(dialogueSFX);
			}

			if (currentFrame == 3)
			{
				if (pressSpaceCounter == 1)
				{
					TextSystem.ResetDialogue(InvitationDialogue1.ID);
                }
				else
				{
					TextSystem.StartDialogue(InvitationDialogue1.ID);
				}
			}

            if (currentFrame == 4)
            {
				if (pressSpaceCounter == 1)
				{
					TextSystem.ResetDialogue(InvitationDialogue2.ID);
				}
				else
				{
                    TextSystem.StartDialogue(InvitationDialogue2.ID);
				}
            }

            if (currentFrame == 5)
            {
				if (pressSpaceCounter == 1)
				{
					TextSystem.ResetDialogue(InvitationDialogue3.ID);
				}
				else
				{
                    TextSystem.StartDialogue(InvitationDialogue3.ID);
				}
            }

            if (currentFrame == 6)
            {
                if (pressSpaceCounter == 1)
                {
                    TextSystem.ResetDialogue(InvitationDialogue4.ID);
                }
                else
                {
                    TextSystem.StartDialogue(InvitationDialogue4.ID);
                }
            }

            if (currentFrame == 7)
            {
                if (pressSpaceCounter == 1)
                {
                    TextSystem.ResetDialogue(InvitationDialogue5.ID);
                }
                else
                {
                    TextSystem.StartDialogue(InvitationDialogue5.ID);
                }
            }

            // Close Game
            if (InputSystem.GetKeyHold(InputKeys.Escape))
			{
				Game.CloseGame();
			}

			// Ignore if last frame alr
			if (currentFrame >= frames.Count) return;

			// Timer to go to next frame
			if (currentTime > 0)
			{
				currentTime -= Time.deltaTime;
			}

            if (pressSpaceCounter == 1)
            {
                pressedSpaceTwice = InputSystem.GetKeyPress(InputKeys.Space);
                if (pressedSpaceTwice)
                {
                    Debug.Log("pressed space twice");
                }
            }

            if (!endCutscene)
			{
				//Every frame will go through this if statement (when its going to the next frame)
				if ((pressedSpace || pressA) || (currentFrame < frames.Count && !forcedScenes.Contains(frames[currentFrame].name) && 
                                                 frames[currentFrame].GetComponent<VFX_FadeIn>().DoneFading() && currentTime <= 0))
				{
					//Debug.Log("current frame: " + currentFrame);
					frames[currentFrame].GetComponent<VFX_FadeIn>().ForceComplete();
					//Debug.Log("next frame");

					++pressSpaceCounter;
                    if (!(currentFrame >= 3 && currentFrame <= 7))
                    {
                        pressedSpaceTwice = true;
                        Debug.Log("pressed space");
                    }

                    if (pressedSpace && pressedSpaceTwice) 
					{ 
						++currentFrame;
						pressedSpaceTwice = false;
						pressSpaceCounter = 0;
					}

					if (ECSManager.IsValidEntity(dialogueSFX))
						AS.Stop(dialogueSFX);

					//currentframe == 3/4/5/6/7, SpaceToContinueBlack
					//letter frames + last frame + out of bounds frame will go through this if statement
					if (currentFrame >= frames.Count - 1 || forcedScenes.Contains(frames[currentFrame].name))
					{
						if (currentFrame >= 3 && currentFrame <= 7)
						{
							SpaceToContinueBlack.SetActive(true);
						}
						else
							SpaceToContinue.SetActive(true);
					}
					//disable SpaceToContinue
					else if (SpaceToContinue.GetActive() || SpaceToContinueBlack.GetActive())
					{
						SpaceToContinue.SetActive(false);
						SpaceToContinueBlack.SetActive(false);
					}

					if (currentFrame >= frames.Count) return;

					// Check if deactivate all frames if counted as next scene
					//Every new frame that appears
					if (nextScenes.Contains(frames[currentFrame].name))
					{
						FadeAllActive();
						currentTime = delayScene;
					}
					else
					{
						currentTime = delayFrame;
					}

					// Fade In next frame
					frames[currentFrame].SetActive(true);
					frames[currentFrame].GetComponent<VFX_FadeIn>().FadeIn();
				}
			}
		}

		private void FadeAllActive()
		{
			for (int i = 0; i < frames.Count; ++i)
			{
				//fade out all the previous frames
				if (frames[i].GetComponent<VFX_FadeIn>().DoneFading())
				{
					frames[i].GetComponent<VFX_FadeOut>().FadeOut();
				}
			}
		}
	}
}
