#include "pch.h"
#include "Demo/Demo.h"

#include <random>

namespace TRE
{
	static int i = 0;
	static std::default_random_engine generator;
	static std::uniform_real_distribution<float> distribution(-50.f, 50.f);
	static std::uniform_real_distribution<float> distributionZ(0.f, 500.f);
	void Demo::SpawnObject()
	{
		auto textureHandle = Resource::GetGUIDFromHex("474d70e35d64e711"); //diffuse
		auto textureHandle2 = Resource::GetGUIDFromHex("d3464713e4f44bee"); //normal
		auto textureHandle3 = Resource::GetGUIDFromHex("8a0c8bee2a64d76b"); //roughness
		auto textureHandle4 = Resource::GetGUIDFromHex("13392e8301ebb46"); //AO
		auto skullHandle = Resource::GetGUIDFromHex("b1d2057915001876"); //skull
		//auto matHandle = Resource::GetGUIDFromHex("74b283e6a2bed9d8");
		auto vertHandle = 3;
		auto fragHandle = 4;
		auto matHandle = Resource::GenerateGUID();

		auto VertShader = ResourceManager::Instance().GetResource<Shader>(vertHandle);
		auto FragShader = ResourceManager::Instance().GetResource<Shader>(fragHandle);
		std::unique_ptr<Material> mat = std::make_unique<Material>(VertShader, FragShader);
		mat->SetHandle(matHandle);
		mat->SetTextures(ResourceManager::Instance().GetResource<VulkanTexture>(textureHandle));
		mat->SetTextures(ResourceManager::Instance().GetResource<VulkanTexture>(textureHandle2));
		mat->SetTextures(ResourceManager::Instance().GetResource<VulkanTexture>(textureHandle3));
		mat->SetTextures(ResourceManager::Instance().GetResource<VulkanTexture>(textureHandle4));
		ResourceManager::Instance().AddResource(std::move(mat));

		auto transformSystem = ECSSystemManager::Instance().GetSystem<TransformSystem>();
		auto meshRendererSystem = ECSSystemManager::Instance().GetSystem<MeshRendererSystem>();

		Entity test = ECSManager::Instance().CreateEntity();
		test->GetComponent<Properties>().m_Name = "Test " + std::to_string(i++);
		transformSystem->SetPosition(test, glm::vec3(distribution(generator), distribution(generator), distributionZ(generator)));
		transformSystem->SetScale(test, glm::vec3(0.2f, 0.2f, 0.2f));
		transformSystem->SetRotation(test, glm::vec3(0, 180.f, 0));
		test->AddComponent<MeshRenderer>();
		meshRendererSystem->SetMeshRenderer(test, ResourceManager::Instance().GetResource<RenderObject>(skullHandle));
		meshRendererSystem->SetMaterial(test, ResourceManager::Instance().GetResource<Material>(matHandle));
	}
}
