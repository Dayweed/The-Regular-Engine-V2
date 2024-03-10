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

			// find first (external) button
			Entity firstButtonEntity = ECSManager.FindEntityByName("Button");

			// add that button first, if it exists
			if (firstButtonEntity.ID != 0)
				buttonList.Add(firstButtonEntity.GetComponent<Button>());

			// Take note: The buttons that have a OnOffPlatform as their parent
			// need to be un-prefab-ed! For some reason!!
			GetRemainingButtons();

			// assert that the number of platforms and buttons should be the same!!!
			if (buttonList.Count != platformList.Count)
			{
				string className = this.ToString();
				Debug.LogError("[" + className + "] Different number of OnOffPlatforms and Buttons!");
				Debug.LogError("[" + className + "] Did you forget to place the initial button?");
				return;
			}
		}

		public void Update()
		{
			// the number of platforms and buttons should be the same!!!
			if (buttonList.Count != platformList.Count)
				return;

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
	}

	public class SortEntitiesAlphabetically : IComparer<Entity>
	{
		public int Compare(Entity x, Entity y)
		{
			return x.name.CompareTo(y.name);
		}
	}
}
