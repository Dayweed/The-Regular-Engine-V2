#pragma once
#include <string>
#include <fstream>

namespace TRE
{
	class DescriptorFile
	{
	public:
		//Order: SetAssetPath, Generate, Close file
		void GenerateDescriptorFile(const std::string& assetPath, const std::string& descPath);
		void ReadDescriptorFile(const std::string& descriptorPath);

		void SetAssetPath(const std::string& path) { m_AssetPath = path; }
		void SetDescriptorPath(const std::string& path) { m_DescriptorPath = path; }

		const std::string& GetAssetPath() const { return m_AssetPath; }
		const std::string& GetDescriptorPath() const { return m_DescriptorPath; }
	protected:
		virtual void Write() {};
		virtual void Read() {};

	protected:
		std::string		m_AssetPath;
		std::string		m_DescriptorPath;
		std::fstream	m_DescriptorFile;
	private:
		void WriteAssetPath();
		void ReadDescriptorPath();
	};
}