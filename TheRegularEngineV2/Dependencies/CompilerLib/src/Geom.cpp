#include "Geom.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <direct.h>

namespace TRE
{
	void Geom::RunCompiler(std::string descPath)
	{
		const char* geomExe = "..\\Compilers\\GeomCompiler.exe";
		if (std::filesystem::exists(geomExe) == false)
		{
			std::cout << "Error: GeomCompiler.exe does not exist" << std::endl;
			return;
		}

		char currentDir[FILENAME_MAX];
		if (_getcwd(currentDir, sizeof(currentDir)))
		{
			//std::string exePath = currentDir;
			//exePath += "\\";
			//exePath += textureExe;
			std::string exePath = geomExe;

			std::replace(descPath.begin(), descPath.end(), '/', '\\');
			//std::string newDescPath = currentDir;
			//newDescPath += "\\";
			//newDescPath += descPath;
			std::string newDescPath = descPath;

			char command[256];
			snprintf(command, sizeof(command), "\"%s %s\"", exePath.c_str(), newDescPath.c_str());

			int result = system(command);
			if (result != 0)
			{
				std::cout << "Error: GeomCompiler.exe failed to run. Code: " << result << std::endl;
				return;
			}
		}
		else
		{
			std::cout << "Error: could not get current directory" << std::endl;
			return;
		}
	}

	void Geom::Serialize(const std::string& filePath, const std::unique_ptr<Geom> geom, const Skeleton& Skel, const std::vector<Animation>& Animations)
	{
		std::string_view name = filePath;
		name.remove_prefix(name.find_last_of('/') + 1);
		name.remove_suffix(name.size() - name.find_last_of('.'));

		std::ofstream file(filePath, std::ios::binary);

		//file.write(reinterpret_cast<const char*>(&m_Geom->pMesh->Name), sizeof(Geom::Mesh) * m_Geom->nMeshes);
		//file.write(reinterpret_cast<const char*>(&m_Geom->pSubMesh), sizeof(Geom::SubMesh) * m_Geom->nSubMeshes);
		file.write(reinterpret_cast<const char*>(&geom->nPosition), sizeof(std::uint32_t));
		file.write(reinterpret_cast<const char*>(geom->pPosition), sizeof(Geom::Position) * geom->nPosition);
		file.write(reinterpret_cast<const char*>(&geom->nExtras), sizeof(std::uint32_t));
		file.write(reinterpret_cast<const char*>(geom->pExtra), sizeof(Geom::Extra) * geom->nExtras);
		file.write(reinterpret_cast<const char*>(&geom->nIndices), sizeof(std::uint32_t));
		file.write(reinterpret_cast<const char*>(geom->pIndices), sizeof(std::uint32_t) * geom->nIndices);

		int Animated = geom->m_IsAnimated ? 1 : 0;
		file.write(reinterpret_cast<const char*>(&Animated), sizeof(int));
		if (geom->m_IsAnimated)
		{
			std::cout << "Is Animated" << std::endl;
			std::cout << "Serialize bones" << std::endl;
			file.write(reinterpret_cast<const char*>(&geom->nBones), sizeof(std::uint32_t));
			file.write(reinterpret_cast<const char*>(geom->pBone), sizeof(BoneInfluence) * geom->nBones);

			std::cout << "Serialize Skeleton" << std::endl;
			uint32_t NumberOfBones = Skel.m_Bones.size();
			std::cout << "Skeleton bone size: " << NumberOfBones << std::endl;
			file.write(reinterpret_cast<const char*>(&NumberOfBones), sizeof(std::uint32_t));

			for (int x = 0; x < NumberOfBones; x++)
			{
				uint32_t NameSize = Skel.m_Bones[x].m_Name.size();
				file.write(reinterpret_cast<const char*>(&NameSize), sizeof(std::uint32_t));
				file.write(reinterpret_cast<const char*>(Skel.m_Bones[x].m_Name.data()), NameSize);
				file.write(reinterpret_cast<const char*>(&Skel.m_Bones[x].m_InvBind), sizeof(glm::mat4));
				file.write(reinterpret_cast<const char*>(&Skel.m_Bones[x].m_NeutalPose), sizeof(glm::mat4));
				file.write(reinterpret_cast<const char*>(&Skel.m_Bones[x].m_iParent), sizeof(int));
			}

			std::cout << "Serialize Animation" << std::endl;
			uint32_t NumberOfAnimations = Animations.size();
			std::cout << "Number of animations size: " << NumberOfAnimations<< std::endl;
			file.write(reinterpret_cast<const char*>(&NumberOfAnimations), sizeof(std::uint32_t));

			for (int x = 0; x < NumberOfAnimations; x++)
			{
				uint32_t StringSize = Animations[x].m_Name.size();
				file.write(reinterpret_cast<const char*>(&StringSize), sizeof(uint32_t));
				file.write(reinterpret_cast<const char*>(&Animations[x].m_Name), StringSize);
				file.write(reinterpret_cast<const char*>(&Animations[x].m_FPS), sizeof(int));
				file.write(reinterpret_cast<const char*>(&Animations[x].m_TimeLength), sizeof(float));
				
				uint32_t SizeBoneKeyFrames = Animations[x].m_BoneKeyFrames.size();
				file.write(reinterpret_cast<const char*>(&SizeBoneKeyFrames), sizeof(uint32_t));

				for (int y = 0; y < SizeBoneKeyFrames; y++)
				{
					uint32_t ScaleSize = Animations[x].m_BoneKeyFrames[y].m_Scale.size();
					uint32_t RotateSize = Animations[x].m_BoneKeyFrames[y].m_Rotate.size();
					uint32_t TranslateSize = Animations[x].m_BoneKeyFrames[y].m_Translate.size();
					
					file.write(reinterpret_cast<const char*>(&ScaleSize), sizeof(std::uint32_t));
					file.write(reinterpret_cast<const char*>(&RotateSize), sizeof(std::uint32_t));
					file.write(reinterpret_cast<const char*>(&TranslateSize), sizeof(std::uint32_t));

					file.write(reinterpret_cast<const char*>(Animations[x].m_BoneKeyFrames[y].m_Scale.data()), sizeof(glm::vec3) * ScaleSize);
					file.write(reinterpret_cast<const char*>(Animations[x].m_BoneKeyFrames[y].m_Rotate.data()), sizeof(glm::quat) * RotateSize);
					file.write(reinterpret_cast<const char*>(Animations[x].m_BoneKeyFrames[y].m_Translate.data()), sizeof(glm::vec3) * TranslateSize);
				}
			}
		}

		file.close();
	}

	std::unique_ptr<Geom> Geom::Deserialize(const std::string& filePath)
	{
		auto geom = std::make_unique<Geom>();

		std::filesystem::path path = filePath;
		if (std::filesystem::exists(filePath))
		{
			std::ifstream file(filePath, std::ios::binary);
			if (file.is_open())
			{
				file.seekg(0, std::ios::end);
				std::size_t size = file.tellg();
				file.seekg(0, std::ios::beg);

				char* buffer = new char[size];
				file.read(buffer, size);
				file.close();

				std::size_t offset = 0;
				//geom->pMesh = reinterpret_cast<Geom::Mesh*>(buffer);
				//geom->pSubMesh = reinterpret_cast<Geom::SubMesh*>(buffer + sizeof(Geom::Mesh) * geom->nMeshes);
				geom->nPosition = *reinterpret_cast<std::uint32_t*>(buffer + offset);
				offset += sizeof(std::uint32_t);
				
				geom->pPosition = new Geom::Position[geom->nPosition];
				memcpy(geom->pPosition, buffer + offset, sizeof(Geom::Position) * geom->nPosition);
				offset += sizeof(Geom::Position) * geom->nPosition;
				
				geom->nExtras = *reinterpret_cast<std::uint32_t*>(buffer + offset);
				geom->pExtra = new Geom::Extra[geom->nExtras];
				offset += sizeof(std::uint32_t);
				
				memcpy(geom->pExtra, buffer + offset, sizeof(Geom::Extra) * geom->nExtras);
				offset += sizeof(Geom::Extra) * geom->nExtras;
				
				geom->nIndices = *reinterpret_cast<std::uint32_t*>(buffer + offset);
				geom->pIndices = new std::uint32_t[geom->nIndices];
				offset += sizeof(std::uint32_t);

				memcpy(geom->pIndices, buffer + offset, sizeof(std::uint32_t) * geom->nIndices);
				offset += sizeof(std::uint32_t) * geom->nIndices;

				int Animated;
				memcpy(&Animated, buffer + offset, sizeof(int));
				offset += sizeof(int);

				geom->m_IsAnimated = (Animated == 1) ? true : false;

				if (geom->m_IsAnimated)
				{
					std::cout << "Deserialize Bones" << std::endl;
					geom->nBones = *reinterpret_cast<std::uint32_t*>(buffer + offset);
					geom->pBone = new BoneInfluence[geom->nBones];
					offset += sizeof(std::uint32_t);
					memcpy(geom->pBone, buffer + offset, sizeof(BoneInfluence) * geom->nBones);
					offset += sizeof(BoneInfluence) * geom->nBones;

					std::cout << "Deserialize Skeleton" << std::endl;
					uint32_t SkelBone = *reinterpret_cast<std::uint32_t*>(buffer + offset);
					offset += sizeof(std::uint32_t);
					geom->m_Skeleton.m_Bones.resize(SkelBone);

					for (int x = 0; x < SkelBone; x++)
					{
						uint32_t NameSize = *reinterpret_cast<std::uint32_t*>(buffer + offset);
						offset += sizeof(std::uint32_t);
						geom->m_Skeleton.m_Bones[x].m_Name.resize(NameSize);

						memcpy(geom->m_Skeleton.m_Bones[x].m_Name.data(), buffer + offset, NameSize);
						offset += NameSize;

						geom->m_Skeleton.m_Bones[x].m_InvBind = *reinterpret_cast<glm::mat4*>(buffer + offset);
						offset += sizeof(glm::mat4);

						geom->m_Skeleton.m_Bones[x].m_NeutalPose = *reinterpret_cast<glm::mat4*>(buffer + offset);
						offset += sizeof(glm::mat4);

						geom->m_Skeleton.m_Bones[x].m_iParent = *reinterpret_cast<int*>(buffer + offset);
						offset += sizeof(int);
					}

					std::cout << "Deserialize Animations" << std::endl;
					uint32_t AnimationsNumber = *reinterpret_cast<std::uint32_t*>(buffer + offset);
					offset += sizeof(std::uint32_t);
					geom->m_Animation.resize(AnimationsNumber);

					for (int x = 0; x < AnimationsNumber; x++)
					{
						uint32_t StringSize = *reinterpret_cast<std::uint32_t*>(buffer + offset);
						offset += sizeof(std::uint32_t);
						geom->m_Animation[x].m_Name.resize(StringSize);
						memcpy(geom->m_Animation[x].m_Name.data(), buffer + offset, StringSize);
						offset += StringSize;

						geom->m_Animation[x].m_FPS = *reinterpret_cast<int*>(buffer + offset);
						offset += sizeof(int);

						geom->m_Animation[x].m_TimeLength = *reinterpret_cast<float*>(buffer + offset);
						offset += sizeof(float);

						uint32_t BoneKeyFramesSize = *reinterpret_cast<std::uint32_t*>(buffer + offset);
						geom->m_Animation[x].m_BoneKeyFrames.resize(BoneKeyFramesSize);
						offset += sizeof(std::uint32_t);

						for (int y = 0; y < BoneKeyFramesSize; y++)
						{
							uint32_t ScaleSize = *reinterpret_cast<std::uint32_t*>(buffer + offset);
							offset += sizeof(std::uint32_t);

							uint32_t RotateSize = *reinterpret_cast<std::uint32_t*>(buffer + offset);
							offset += sizeof(std::uint32_t);

							uint32_t TranslateSize = *reinterpret_cast<std::uint32_t*>(buffer + offset);
							offset += sizeof(std::uint32_t);

							geom->m_Animation[x].m_BoneKeyFrames[y].m_Scale.resize(ScaleSize);
							geom->m_Animation[x].m_BoneKeyFrames[y].m_Rotate.resize(RotateSize);
							geom->m_Animation[x].m_BoneKeyFrames[y].m_Translate.resize(TranslateSize);

							memcpy(geom->m_Animation[x].m_BoneKeyFrames[y].m_Scale.data(), buffer + offset, ScaleSize * sizeof(glm::vec3));
							offset += sizeof(glm::vec3) * ScaleSize;
							memcpy(geom->m_Animation[x].m_BoneKeyFrames[y].m_Rotate.data(), buffer + offset, RotateSize * sizeof(glm::quat));
							offset += sizeof(glm::quat) * RotateSize;
							memcpy(geom->m_Animation[x].m_BoneKeyFrames[y].m_Translate.data(), buffer + offset, TranslateSize * sizeof(glm::vec3));
							offset += sizeof(glm::vec3) * TranslateSize;
						}
					}
				}

				delete[] buffer;
			}
			else
			{
				std::cout << "Error: could not open file: " << filePath << std::endl;
			}
		}
		else
		{
			std::cout << "Error: file does not exist: " << filePath << std::endl;
		}

		return geom;
	}
}