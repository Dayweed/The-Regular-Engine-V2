using GlmSharp;
using System.Collections.Generic;

namespace TRE
{
	struct CameraData
	{
		public vec3 Position;
		public vec3 Rotation;
		public float Duration;
	};

	public class CameraTransitions : Entity
	{
		private List<CameraData> cameraDataList;
		private int trackIndex = 0;
		private float elapsed;

		public bool preTransitioned = false;

		public void Init()
		{
			cameraDataList = new List<CameraData>();
			trackIndex = 0;
			elapsed = 0f;
			preTransitioned = false;
		}

		public void AddCameraData(vec3 position, vec3 rotation, float duration)
		{
			CameraData data = new CameraData
			{
				Position = position,
				Rotation = rotation,
				Duration = duration
			};
			cameraDataList.Add(data);
		}

		public void ClearCameraData()
		{
			cameraDataList.Clear();
		}

		public void GetCurrentData(out vec3 position, out vec3 rotation, out float duration)
		{
			position = cameraDataList[trackIndex].Position;
			rotation = cameraDataList[trackIndex].Rotation;
			duration = cameraDataList[trackIndex].Duration;
		}

		public void PreTransition(float dt, out bool toTransition)
		{
			if (InputSystem.GetKeyPress(InputKeys.Escape))
			{
				preTransitioned = true;
				toTransition = false;
				return;
			}

			if (preTransitioned)
			{
				toTransition = false;
				return;
			}

			//Empty or done
			if (cameraDataList.Count == 0 || trackIndex >= cameraDataList.Count)
			{
				preTransitioned = true;
				toTransition = false;
				return;
			}

			//First instance
			if (elapsed == 0f)
			{
				toTransition = true;
				elapsed += dt;
				return;
			}

			if (elapsed >= cameraDataList[trackIndex].Duration)
			{
				elapsed = 0f;
				++trackIndex;
				toTransition = true;

				if (trackIndex >= cameraDataList.Count)
				{
					preTransitioned = true;
				}
				return;
			}

			toTransition = false;
			elapsed += dt;
		}
	}
}
