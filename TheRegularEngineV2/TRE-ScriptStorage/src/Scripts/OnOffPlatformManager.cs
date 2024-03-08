using System;
using System.Collections.Generic;

namespace TRE
{

	public class OnOffPlatformManager : Entity
	{
		List<Entity> platformList = new List<Entity>();

		public void Start()
		{
			int childCount = parenting.GetTotalChildren();
			platformList.Capacity = childCount;

			for (int i = 0; i < childCount; ++i)
				platformList.Add(parenting.GetChild(i));

			platformList.Sort(new SortAlphabetically());

			print("Here are the children in alphabetical order:");
			foreach (var child in platformList)
				print(child.name);

			// var comp = true ? platformList[0].GetComponent<OnOffPlatform>() : platformList[0].GetComponent<OnOffPlatformOpposite>();
		}

		public void Update()
		{

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
