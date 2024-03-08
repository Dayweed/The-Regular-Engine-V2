using System;
using System.Collections.Generic;

namespace TRE
{

	public class OnOffPlatformManager : Entity
	{
		List<OnOffPlatform> platformList = new List<OnOffPlatform>();

		public void Start()
		{
			int childCount = parenting.GetTotalChildren();
			platformList.Capacity = childCount;

			for (int i = 0; i < childCount; ++i)
				platformList.Add(parenting.GetChild(i).GetComponent<OnOffPlatform>());

			platformList.Sort(new SortAlphabetically());

			print("Here are the children in alphabetical order:");
			foreach (var child in platformList)
				print(child.name);
		}

		public void Update()
		{
			FlipPlatformsInSeries();
		}

		bool tempBehaviourMode = false;

		void FlipPlatformsInSeries()
		{
			// obtain all the states of the on/off platforms
			bool[] platformStates = new bool[platformList.Count];
			bool isAllActive = true;
			bool isAllInactive = false;

			for (int i = 0; i < platformStates.Length; ++i)
			{
				bool state = platformList[i].GetComponent<OnOffPlatform>().GetPlatformState();
				platformStates[i] = state;
				isAllActive &= state; // if there's a single false, the result will be false
				isAllInactive |= state; // if there's a single true, the result will be true
			}

			isAllInactive = !isAllInactive; // flip the result to match the variable name's meaning

			if (isAllInactive)
				tempBehaviourMode = false;
			if (isAllActive)
				tempBehaviourMode = true;

			if (!tempBehaviourMode)
			{
				for (int i = 0; i < platformStates.Length; ++i)
				{
					// raise platforms one by one
					if (platformStates[i] == false)
					{
						platformList[i].SetPlatformState(true);
						break;
					}
				}
			}
			else
			{
				for (int i = 0; i < platformStates.Length; ++i)
				{
					// lower platforms one by one
					if (platformStates[i] == true)
					{
						platformList[i].SetPlatformState(false);
						break;
					}
				}
			}
		}

		void print(string str)
		{
			Console.Write("[{0}]\t{1}", this.name, str + "\n");
		}
	}

	public class SortAlphabetically : IComparer<Entity>
	{
		public int Compare(Entity x, Entity y)
		{
			return x.name.CompareTo(y.name);
		}
	}
}
