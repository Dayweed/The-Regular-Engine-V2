#pragma once
#include "EventBase.h"
#include "TREIncludes.h"
#include <string>
#include <chrono>

#define CONSOLE_DEBUG_WARN	"[WARNING] "
#define CONSOLE_DEBUG_ERROR "[ERROR] "

namespace TRE
{
	struct ConsoleStartEvent : Event
	{
		bool m_IsSimulating{ false };
		ConsoleStartEvent() = delete;
		ConsoleStartEvent(bool gameIsSimulating) : m_IsSimulating(gameIsSimulating) {}
	};

	struct ConsoleDebugEvent : Event
	{
		std::string m_Msg;
		ConsoleDebugEvent(std::string&& msg) : m_Msg(std::move(msg)) {}
	};
	//To use the event just include this file and publish the event like this:
	//EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ "Testing Key inputs here" });

	struct SendTimeTakenEvent : Event
	{
		std::unordered_map<std::string, Timer*> m_Timers{};
		SendTimeTakenEvent() = delete;
		SendTimeTakenEvent(std::unordered_map<std::string, Timer*> timer) : m_Timers(timer) {}
	};

	struct ToggleRunEvent : Event
	{
		bool m_Playing{ false };
		bool m_IsSimulating{ false };
		ToggleRunEvent() = delete;
		ToggleRunEvent(bool playing, bool gameIsSimulating) : m_Playing(playing), m_IsSimulating(gameIsSimulating) {}
	};

	struct ResetSceneEvent : Event
	{
		bool m_Nth{ false };
		ResetSceneEvent() = delete;
		ResetSceneEvent(bool nth) : m_Nth(nth) {}
	};

	struct GridAndSnapEvent : Event
	{
		float m_PosIncrement{};
		float m_RotIncrement{};
		float m_ScaleIncrement{};
		GridAndSnapEvent() = delete;
		GridAndSnapEvent(float pos, float rot, float scale) : m_PosIncrement(pos), m_RotIncrement(rot), m_ScaleIncrement(scale) {}
	};

	struct LocalGloalGizmoEvent : Event
	{
		bool m_IsLocal{ false };
		LocalGloalGizmoEvent() = delete;
		LocalGloalGizmoEvent(bool local) : m_IsLocal(local) {}
	};

	struct GizmoOperationEvent : Event
	{
		int m_Operation{};
		GizmoOperationEvent() = delete;
		GizmoOperationEvent(int operation) : m_Operation(operation) {}
	};

	struct EditorCameraEvent : Event
	{
		float m_PanSpeed{};
		float m_ZoomSensitivity{};
		float m_RotationSensitivity{};
		EditorCameraEvent() = delete;
		EditorCameraEvent(float pan, float zoom, float rot) : m_PanSpeed(pan), m_ZoomSensitivity(zoom), m_RotationSensitivity(rot) {}
	};;

	struct AssetSelectorEvent : Event
	{
		std::string m_AssetName;
		enum class AssetType
		{
			Unknown = 0, Texture, Model, Shader, Material, Scene, Audio, Font, Script, Particle, Prefab
		};
		AssetType m_AssetType;
		AssetSelectorEvent() = delete;
		AssetSelectorEvent(std::string assetName, AssetType assetType) : m_AssetName(assetName), m_AssetType(assetType) {}
	};

	struct AssetPanelEvent : Event
	{
		bool m_ShowAssetPanel{ false };
		AssetPanelEvent() = delete;
		AssetPanelEvent(bool showAssetPanel) : m_ShowAssetPanel(showAssetPanel) {}
	};

	struct CollisionMatrixEvent : Event
	{
		bool m_ShowCollisionMatrixPanel = false;
		CollisionMatrixEvent(bool showPanel) : m_ShowCollisionMatrixPanel(showPanel) {}
	};
}
