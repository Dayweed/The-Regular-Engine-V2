#pragma once
#include "Panel.h"
#include "EventSystem/Events/EditorEvent.h"

namespace TRE
{
	class CollisionMatrixPanel : public Panel
	{
	public:
		void Init() override;
		void Update() override;
		void Shutdown() override;
	private:
		bool m_ShowPanel = false;
		void OnCollisionMatrixEvent(const CollisionMatrixEvent& event);
		void HelpMarker(const char* prompt, const char* helpText) const;
		void HoverOverText(const char* text) const;
		std::string CreateLayerAcronym(const int index);
	};
}
