using System;
using System.Threading;
using Coroutine;

namespace TRE
{
	public class Main
	{
		public void Start()
		{

		}

		public void Update()
		{
			var lastTime = DateTime.Now;
			while (true)
			{
				var currtime = DateTime.Now;
				CoroutineHandler.Tick(currtime - lastTime);
				lastTime = currtime;
				Thread.Sleep(1);
			}
		}
	}
}
