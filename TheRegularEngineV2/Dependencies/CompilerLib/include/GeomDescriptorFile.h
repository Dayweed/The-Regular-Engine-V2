#pragma once
#include "DescriptorFile.h"
#include "Geom.h"

namespace TRE
{
	class GeomDescriptorFile : public DescriptorFile
	{
	public:
		void SetPosition(const glm::vec3& position) { m_Position = position; }
		void SetRotation(const glm::vec3& rotation) { m_Rotation = rotation; }
		void SetScale(const glm::vec3& scale) { m_Scale = scale; }
		void SetOptimize(const bool optimize) { m_Optimize = optimize; }
		//void SetMeshRenameBool(const bool meshRename) { m_MeshRename = meshRename; }
		//void SetMeshName(const std::string& meshName) { m_MeshName = meshName; }

		const glm::vec3& GetPosition() const { return m_Position; }
		const glm::vec3& GetRotation() const { return m_Rotation; }
		const glm::vec3& GetScale() const { return m_Scale; }
		const bool GetOptimize() const { return m_Optimize; }
		//const bool GetMeshRenameBool() const { return m_MeshRename; }
		//const std::string& GetMeshName() const { return m_MeshName; }
	protected:
		void Write() override;
		void Read() override;
	private:
		glm::vec3 m_Position{ 0,0,0 };
		glm::vec3 m_Rotation{ 0,0,0 };
		glm::vec3 m_Scale{ 1,1,1 };
		bool m_Optimize{ true };
		//bool m_MeshRename{ false };
		//std::string m_MeshName;
	};
}