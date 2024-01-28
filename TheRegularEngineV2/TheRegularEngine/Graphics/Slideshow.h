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

	public:
		int m_Slices = 1;
		float m_Speed = 1.f;

	private:
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		int m_CurrentSlice = 0;

	public:
		property_vtable()

		friend void to_json(nlohmann::json& j, const SlideshowComponent& t)
		{
			j = nlohmann::json
			{
				{ "Slices", t.m_Slices },
				{ "Speed", t.m_Speed }
			};
		}

		friend void from_json(const nlohmann::json& j, SlideshowComponent& t)
		{
			if (j.contains("Slices"))
			{
				t.m_Slices = j.at("Slices").get<int>();
			}
			if (j.contains("Speed"))
			{
				t.m_Speed = j.at("Speed").get<float>();
			}
		}
	};
}

property_begin(TRE::SlideshowComponent)
{
	/*property_var_fnbegin("Texture", resource_ref)
	{
		if (isRead)
		{
			if (Self.m_Texture)
				InOut.m_Value = Self.m_Texture->GetHandle();
			else
				InOut.m_Value = 0;
		}
		else
		{
			if (InOut.m_Value)
				Self.m_Texture = TRE::ResourceManager::Instance().GetResource<TRE::VulkanTexture>(InOut.m_Value);
			else
				Self.m_Texture = nullptr;
		}

	} property_var_fnend(),
		property_var(m_IsVisible),
		property_var_fnbegin("Color", Color)
	{
		if (isRead)
		{
			InOut.m_Value = Self.m_Color;
		}
		else
		{
			Self.m_Color = InOut.m_Value;
		}
	} property_var_fnend(),
		property_var(m_Width),
		property_var(m_Height),
		property_var(m_RenderLayer)*/

} property_vend_h(TRE::SlideshowComponent)