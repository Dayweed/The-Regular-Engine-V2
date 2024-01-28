#pragma once
#include "Core/ECS.h"
#include "VertexBuffer.h"

namespace TRE
{
	class SlideshowComponent : property::base
	{
	public:
		SlideshowComponent();
		void GenerateVertexBuffer();
		std::shared_ptr<VertexBuffer> GetCurrentVertexBuffer();
		int VertexContainerSize();

	public:
		int m_Slices = 1;
		int m_CurrentSlice = 0;
		float m_AnimationDuration = 1.f;
		float m_ElapsedTime = 0.f;

	private:
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;

	public:
		property_vtable()

		friend void to_json(nlohmann::json& j, const SlideshowComponent& t)
		{
			j = nlohmann::json
			{
				{ "Slices", t.m_Slices },
				{ "Animation Duration", t.m_AnimationDuration }
			};
		}

		friend void from_json(const nlohmann::json& j, SlideshowComponent& t)
		{
			if (j.contains("Slices"))
			{
				t.m_Slices = j.at("Slices").get<int>();
			}
			if (j.contains("Animation Duration"))
			{
				t.m_AnimationDuration = j.at("Animation Duration").get<float>();
			}
		}
	};

	class SlideshowSystem : public ECSSystem
	{
	public:
		void LateUpdate() override;
	};
}

property_begin(TRE::SlideshowComponent)
{
	property_var(m_Slices),
	property_var(m_AnimationDuration).Name("Total Duration")
} property_vend_h(TRE::SlideshowComponent)