using System;
using System.Collections.Generic;

namespace TRE
{
	public class OnOffPlatformManager : Entity
	{
		List<OnOffPlatform> platformList = new List<OnOffPlatform>();
		List<Button> buttonList = new List<Button>();

		public void Start()
		{
			platformList = GetPlatforms();
			platformList.Sort(new SortEntitiesAlphabetically());

			// if the name has a digit at the end, let it be the number of the initial button
			string nameLastChar = name.Substring(name.Length - 1);
			if (int.TryParse(nameLastChar, out int result))
			{
				// find first (external) button
				Entity firstButtonEntity = ECSManager.FindEntityByName("Button_" + result);

				// add that button first, if it exists
				if (firstButtonEntity.ID != 0)
					buttonList.Add(firstButtonEntity.GetComponent<Button>());
			}
			else
			{
				string message = "[" + this.ToString() + "] Entity \"" + name +
					"\" did not have a digit at the back of its name, " +
					"no initial buttons will be searched for.";
				PrintError(message);
			}

			// Take note: The buttons that have a OnOffPlatform as their parent
			// need to be un-prefab-ed! For some reason!!
			GetRemainingButtons();

			// assert that the number of platforms and buttons should be the same!!!
			if (buttonList.Count != platformList.Count)
			{
				string formatString = "[{0}] Different number of OnOffPlatforms({1}) and Buttons({2})!\n" +
					"[{0}] Did you forget to place the initial button?";
				string message = string.Format(formatString, this.ToString(), platformList.Count, buttonList.Count);
				PrintError(message);

				return;
			}
		}

		public void Update()
		{
			if (InputSystem.GetKeyPress(InputKeys.R))
			{
				MakeAllPlatformsInactive();
				return;
			}

			// the number of platforms and buttons should be the same!!!
			if (buttonList.Count != platformList.Count)
				return;

			// switch 0 should lower x and raise 0
			// switch 1 should lower x and raise 1
			// switch 2 should lower 0 and raise 2
			// switch 3 should lower 1 and raise 3
			// etc...

			for (int i = 0; i < buttonList.Count; ++i)
			{
				Button button = buttonList[i];
				int lowerIndex = i - 2;
				bool lowerIndexInBounds = 0 <= lowerIndex && lowerIndex <= buttonList.Count;
				// int raiseIndex = i;
				// bool raiseIndexInBounds = 0 <= raiseIndex && raiseIndex <= buttonList.Count;

				if (button.GetIsButtonPressed())
				{
					if (lowerIndexInBounds)
						platformList[lowerIndex].SetPlatformState(false);
					platformList[i].SetPlatformState(true);
				}
			}
		}

		List<OnOffPlatform> GetPlatforms()
		{
			int childCount = parenting.GetTotalChildren();
			List<OnOffPlatform> list = new List<OnOffPlatform>(childCount);

			for (int i = 0; i < childCount; ++i)
				list.Add(parenting.GetChild(i).GetComponent<OnOffPlatform>());

			return list;
		}

		void GetRemainingButtons()
		{
			foreach (OnOffPlatform platform in platformList)
			{
				int childrenSize = platform.parenting.GetTotalChildren();

				for (int i = 0; i < childrenSize; ++i)
				{
					Entity child = platform.parenting.GetChild(i);

					if (child.HasComponent<Button>())
					{
						buttonList.Add(child.GetComponent<Button>());
						break;
					}
				}
			}
		}

		void MakeAllPlatformsInactive()
		{
			foreach (OnOffPlatform platform in platformList)
			{
				platform.SetPlatformState(false);
				// platform.ResetPlatform();
			}
		}

		void OldBehaviour()
		{
			for (int i = 0; i < buttonList.Count; ++i)
			{
				// Thingy: <where it is spatially>
				// Platforms:   0 1 2 3 ...
				// Buttons:   0 1 2 3 ...

				OnOffPlatform platform = platformList[i];
				// Button buttonForPlatform = buttonList[i];
				// Button buttonOnPlatform = buttonList[i + 1];

				// Only players can press buttons down.
				// Hence, Button pressed -> Player on *prior* platform.
				// Therefore, Button *ahead by one* is pressed -> Player on platform.

				bool isPlayerOnPlatform;
				if (i == buttonList.Count - 1) // the last platform will never have its own button
					isPlayerOnPlatform = platform.isCollidingWithPlayer;
				else
					isPlayerOnPlatform = buttonList[i + 1].GetIsButtonPressed() || platform.isCollidingWithPlayer;

				// if player is standing on platform and the button FOR that platform is NOT pressed, 
				// DON'T CHANGE ITS STATE!!
				if (isPlayerOnPlatform && !buttonList[i].GetIsButtonPressed())
					continue;

				platform.SetPlatformState(buttonList[i].GetIsButtonPressed());
			}
		}

		void PrintError(string msg)
		{
			Console.WriteLine(msg);
			Debug.LogError(msg);
		}
	}

	public class SortEntitiesAlphabetically : IComparer<Entity>
	{
		public int Compare(Entity x, Entity y)
		{
			return x.name.CompareTo(y.name);
		}
	}
}
