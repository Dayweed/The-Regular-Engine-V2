#include "pch.h"
#include "Core/Logger.h"
#include "GeomCompiler.h"
#include "meshoptimizer.h"

#include <span>
#include <filesystem>

namespace TRE
{
	void GeomCompiler::Compile(const std::string& filename)
	{
		TRE_CORE_INFO("Compiling mesh from: {0}", filename);
		Assimp::Importer importer;

		uint32_t flag = aiProcess_Triangulate                // Make sure we get triangles rather than nvert polygons
			| aiProcess_LimitBoneWeights           // 4 weights for skin model max
			| aiProcess_GenUVCoords                // Convert any type of mapping to uv mapping
			| aiProcess_FindInstances              // search for instanced meshes and remove them by references to one master
			| aiProcess_CalcTangentSpace           // calculate tangents and bitangents if possible (definetly you will meed UVs)
			| aiProcess_RemoveRedundantMaterials   // remove redundant materials
			| aiProcess_FindInvalidData            // detect invalid model data, such as invalid normal vectors
			| aiProcess_FlipUVs                    // flip the V to match the Vulkans way of doing UVs
			;

		m_filePath = filename;
		m_Scene = importer.ReadFile(filename, flag);

		assert(m_Scene != nullptr && "Error loading model");

		if (SanityCheck())
		{
			TRE_CORE_ERROR("Sanity check failed");
			return;
		}

		ImportData();
	}

	void GeomCompiler::Serialize(const std::string& returnPath)
	{
		std::string_view path = returnPath.empty() ? m_filePath : returnPath;
		std::string_view name = path;
		name.remove_prefix(name.find_last_of('/') + 1);
		name.remove_suffix(name.size() - name.find_last_of('.'));

		TRE_CORE_INFO("Serializing mesh to: {0}", path);

		std::ofstream file(path, std::ios::binary);

		//file.write(reinterpret_cast<const char*>(&m_Geom->pMesh->Name), sizeof(Geom::Mesh) * m_Geom->nMeshes);
		//file.write(reinterpret_cast<const char*>(&m_Geom->pSubMesh), sizeof(Geom::SubMesh) * m_Geom->nSubMeshes);
		file.write(reinterpret_cast<const char*>(&m_Geom->nPosition), sizeof(std::uint32_t));
		file.write(reinterpret_cast<const char*>(&m_Geom->pPosition), sizeof(Geom::Position) * m_Geom->nPosition);
		file.write(reinterpret_cast<const char*>(&m_Geom->nExtras), sizeof(std::uint32_t));
		file.write(reinterpret_cast<const char*>(&m_Geom->pExtra), sizeof(Geom::Extra) * m_Geom->nExtras);
		file.write(reinterpret_cast<const char*>(&m_Geom->nIndices), sizeof(std::uint32_t));
		file.write(reinterpret_cast<const char*>(&m_Geom->pIndices), sizeof(std::uint32_t) * m_Geom->nIndices);
		
		file.close();
	}

	void GeomCompiler::Deserialize(const std::string& geomPath)
	{
		TRE_CORE_INFO("Deserializing geom from: {0}", geomPath);
		std::filesystem::path path = geomPath;
		if (std::filesystem::exists(geomPath))
		{
			std::ifstream file(geomPath, std::ios::binary);
			if (file.is_open())
			{
				file.seekg(0, std::ios::end);
				std::size_t size = file.tellg();
				file.seekg(0, std::ios::beg);

				char* buffer = new char[size];
				file.read(buffer, size);
				file.close();
				
				std::size_t offset = 0;
				auto geom = std::make_unique<Geom>();
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
				
				delete[] buffer;

				m_LoadedGeom = std::move(geom);
			}
			else
			{
				TRE_CORE_ERROR("Failed to open file: {0}", geomPath);
			}
		}
		else
		{
			TRE_CORE_ERROR("File does not exist: {0}", geomPath);
		}
	}

	bool GeomCompiler::SanityCheck()
	{
		m_References.resize(m_Scene->mNumMeshes);

		std::function<void(const aiNode& Node)> ProcessNode = [&](const aiNode& Node)
		{
			for (auto i = 0u, end = Node.mNumMeshes; i < end; ++i)
			{
				aiMesh* pMesh = m_Scene->mMeshes[Node.mMeshes[i]];
				(void)pMesh;
				m_References[Node.mMeshes[i]].Nodes.push_back(&Node);
			}

			for (auto i = 0u; i < Node.mNumChildren; ++i)
			{
				ProcessNode(*Node.mChildren[i]);
			}
		};

		ProcessNode(*m_Scene->mRootNode);

		for (auto iMesh = 0u; iMesh < m_Scene->mNumMeshes; ++iMesh)
		{
			const aiMesh& AssimpMesh = *m_Scene->mMeshes[iMesh];
			const auto& Refs = m_References[iMesh].Nodes;

			if (Refs.size() == 0)
			{
				TRE_CORE_WARN("Mesh with no reference in scene");
				return true;
			}

			if (AssimpMesh.HasBones())
			{
				if (Refs.size() > 1)
				{
					TRE_CORE_WARN("Found a skin mesh with multiple references in scene, Unsupported");
					return true;
				}
			}
			else
			{
				if (Refs.size() > 1)
				{
					TRE_CORE_INFO("Mesh with multiple references in scene, Duplicate");
				}
			}
		}

		return false;
	}

	void GeomCompiler::ImportData()
	{
		std::vector<InputMeshPart> MyNodes;
		
		ImportStaticMesh(MyNodes);

		MergeData(MyNodes);

		Optimize(MyNodes);

		auto skinGeom = CreateSkinGeom(Quantize(MyNodes));

		CastToGeom(std::move(skinGeom));
	}

	void GeomCompiler::ImportStaticMesh(std::vector<InputMeshPart>& inputMesh)
	{
		auto ProcessMesh = [&](const aiMesh& AssimpMesh, const aiMatrix4x4& Transform, InputMeshPart& MeshPart, const int iTexCordinates, const int iColors)
		{
			// get the rotation for the normals
			aiQuaternion presentRotation;
			{
				aiVector3D p;
				Transform.DecomposeNoScaling(presentRotation, p);
			}

			MeshPart.Name = AssimpMesh.mName.C_Str();
			MeshPart.MaterialIndex = AssimpMesh.mMaterialIndex;

			MeshPart.Vertices.resize(AssimpMesh.mNumVertices);
			for (unsigned int i = 0; i < AssimpMesh.mNumVertices; ++i)
			{
				FullVertex& Vertex = MeshPart.Vertices[i];

				auto L = Transform * AssimpMesh.mVertices[i];

				Vertex.Position = glm::vec3(static_cast<float>(L.x)
					, static_cast<float>(L.y)
					, static_cast<float>(L.z));

				if (iTexCordinates == -1)
				{
					Vertex.UV = glm::vec2{};
				}
				else
				{
					Vertex.UV = glm::vec2(static_cast<float>(AssimpMesh.mTextureCoords[iTexCordinates][i].x)
						, static_cast<float>(AssimpMesh.mTextureCoords[iTexCordinates][i].y));
				}

				if (iColors == -1)
				{
					Vertex.fColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
					Vertex.Color = IColor(255, 255, 255, 255);
				}
				else
				{
					glm::vec4 RGBA(static_cast<float>(AssimpMesh.mColors[iColors][i].r)
						, static_cast<float>(AssimpMesh.mColors[iColors][i].g)
						, static_cast<float>(AssimpMesh.mColors[iColors][i].b)
						, static_cast<float>(AssimpMesh.mColors[iColors][i].a));

					Vertex.fColor = RGBA;
					Vertex.Color = IColor(static_cast<std::uint8_t>(RGBA.r * 255.0f)
											, static_cast<std::uint8_t>(RGBA.g * 255.0f)
											, static_cast<std::uint8_t>(RGBA.b * 255.0f)
											, static_cast<std::uint8_t>(RGBA.a * 255.0f));
				}

				if (AssimpMesh.HasTangentsAndBitangents())
				{
					const auto T = presentRotation.Rotate(AssimpMesh.mTangents[i]);
					const auto B = presentRotation.Rotate(AssimpMesh.mBitangents[i]);
					const auto N = presentRotation.Rotate(AssimpMesh.mNormals[i]);

					Vertex.fNormal = glm::vec3(N.x, N.y, N.z);
					Vertex.fTangent = glm::vec3(T.x, T.y, T.z);
					Vertex.fBitangent = glm::vec3(B.x, B.y, B.z);

					Vertex.fNormal = glm::normalize(Vertex.fNormal);
					Vertex.fTangent = glm::normalize(Vertex.fTangent);
					Vertex.fBitangent = glm::normalize(Vertex.fBitangent);

					Vertex.Tangent.R = static_cast<std::uint8_t>(static_cast<std::int8_t>(Vertex.fTangent.x < 0 ? std::max(-128, static_cast<int>(Vertex.fTangent.x * 128)) : std::min(127, static_cast<int>(Vertex.fTangent.x * 127))));
					Vertex.Tangent.G = static_cast<std::uint8_t>(static_cast<std::int8_t>(Vertex.fTangent.y < 0 ? std::max(-128, static_cast<int>(Vertex.fTangent.y * 128)) : std::min(127, static_cast<int>(Vertex.fTangent.y * 127))));
					Vertex.Tangent.B = static_cast<std::uint8_t>(static_cast<std::int8_t>(Vertex.fTangent.z < 0 ? std::max(-128, static_cast<int>(Vertex.fTangent.z * 128)) : std::min(127, static_cast<int>(Vertex.fTangent.z * 127))));
					Vertex.Tangent.A = 0;

					assert(AssimpMesh.HasNormals());
					Vertex.Normal.R = static_cast<std::uint8_t>(static_cast<std::int8_t>(Vertex.fNormal.x < 0 ? std::max(-128, static_cast<int>(Vertex.fNormal.x * 128)) : std::min(127, static_cast<int>(Vertex.fNormal.x * 127))));
					Vertex.Normal.G = static_cast<std::uint8_t>(static_cast<std::int8_t>(Vertex.fNormal.y < 0 ? std::max(-128, static_cast<int>(Vertex.fNormal.y * 128)) : std::min(127, static_cast<int>(Vertex.fNormal.y * 127))));
					Vertex.Normal.B = static_cast<std::uint8_t>(static_cast<std::int8_t>(Vertex.fNormal.z < 0 ? std::max(-128, static_cast<int>(Vertex.fNormal.z * 128)) : std::min(127, static_cast<int>(Vertex.fNormal.z * 127))));
					Vertex.Normal.A = static_cast<std::uint8_t>(static_cast<std::int8_t>(glm::dot(glm::cross(Vertex.fTangent, Vertex.fNormal), Vertex.fBitangent) > 0 ? 127 : -128));
				}
				else
				{
					Vertex.Tangent.R = 0xff;
					Vertex.Tangent.G = 0;
					Vertex.Tangent.B = 0;
					Vertex.Tangent.A = 0;

					const auto N = presentRotation.Rotate(AssimpMesh.mNormals[i]);

					Vertex.fNormal = glm::vec3(N.x, N.y, N.z);
					Vertex.fTangent = glm::vec3(1, 0, 0);
					Vertex.fBitangent = glm::vec3(1, 0, 0);

					Vertex.fNormal = glm::normalize(Vertex.fNormal);

					Vertex.Normal.R = static_cast<std::uint8_t>(static_cast<std::int8_t>(Vertex.fNormal.x < 0 ? std::max(-128, static_cast<int>(Vertex.fNormal.x * 128)) : std::min(127, static_cast<int>(Vertex.fNormal.x * 127))));
					Vertex.Normal.G = static_cast<std::uint8_t>(static_cast<std::int8_t>(Vertex.fNormal.y < 0 ? std::max(-128, static_cast<int>(Vertex.fNormal.y * 128)) : std::min(127, static_cast<int>(Vertex.fNormal.y * 127))));
					Vertex.Normal.B = static_cast<std::uint8_t>(static_cast<std::int8_t>(Vertex.fNormal.z < 0 ? std::max(-128, static_cast<int>(Vertex.fNormal.z * 128)) : std::min(127, static_cast<int>(Vertex.fNormal.z * 127))));
					Vertex.Normal.A = 127;
				}
			}

			//
			// Copy the indices
			//
			for (auto i = 0u; i < AssimpMesh.mNumFaces; ++i)
			{
				const auto& Face = AssimpMesh.mFaces[i];
				for (auto j = 0u; j < Face.mNumIndices; ++j)
					MeshPart.Indices.push_back(Face.mIndices[j]);
			}
		};

		std::function<void(const aiNode&, const aiMatrix4x4&)> RecurseScene = [&](const aiNode& Node, const aiMatrix4x4& ParentTransform)
		{
			const aiMatrix4x4 Transform = ParentTransform * Node.mTransformation;
			auto        iBase = inputMesh.size();

			// Collect all the meshes
			inputMesh.resize(iBase + m_Scene->mNumMeshes);
			for (auto i = 0u, end = Node.mNumMeshes; i < end; ++i)
			{
				aiMesh& AssimpMesh = *m_Scene->mMeshes[Node.mMeshes[i]];

				int iTexCordinates, iColor;
				if (ImportGeometryValidateMesh(AssimpMesh, iTexCordinates, iColor)) continue;

				ProcessMesh(AssimpMesh, Transform, inputMesh[iBase++], iTexCordinates, iColor);
			}

			// Make sure the base matches what should be in the vector
			if (iBase != inputMesh.size())
			{
				inputMesh.erase(inputMesh.begin() + iBase, inputMesh.end());
			}

			// Do the children
			for (auto i = 0u; i < Node.mNumChildren; ++i)
			{
				RecurseScene(*Node.mChildren[i], Transform);
			}
		};

		//aiMatrix4x4 L2W = m_DescriptorMatrix;
		aiMatrix4x4 L2W = aiMatrix4x4();
		RecurseScene(*m_Scene->mRootNode, L2W);
	}

	bool GeomCompiler::ImportGeometryValidateMesh(const aiMesh& AssimpMesh, int& iTexture, int& iColor)
	{
		if (AssimpMesh.HasPositions() == false)
		{
			TRE_CORE_INFO("Removing mesh ({0}) without position", AssimpMesh.mName.C_Str());
			return true;
		}

		if (AssimpMesh.HasFaces() == false)
		{
			TRE_CORE_INFO("Removing mesh ({0}) without position", AssimpMesh.mName.C_Str());
			return true;
		}

		if (AssimpMesh.HasNormals() == false)
		{
			TRE_CORE_INFO("Removing mesh ({0}) without normals", AssimpMesh.mName.C_Str());
			return true;
		}

		if (AssimpMesh.HasTangentsAndBitangents() == false)
		{
			TRE_CORE_INFO("Creating tangents and bitangents for mesh ({0})", AssimpMesh.mName.C_Str());
		}

		if (AssimpMesh.GetNumUVChannels() != 1)
		{
			if (AssimpMesh.GetNumUVChannels() == 0)
			{
				TRE_CORE_INFO("No UVs found for mesh ({0})", AssimpMesh.mName.C_Str());
			}
			else
			{
				TRE_CORE_INFO("Too many UV channels found for mesh ({0})", AssimpMesh.mName.C_Str());
			}
		}

		iTexture = [&]()->int
		{
			for (auto i = 0u; i < AssimpMesh.GetNumUVChannels(); ++i)
				if (AssimpMesh.HasTextureCoords(i)) return i;
			return -1;
		}();

		iColor = [&]()->int
		{
			for (auto i = 0u; i < AssimpMesh.GetNumColorChannels(); ++i)
				if (AssimpMesh.HasVertexColors(i)) return i;
			return -1;
		}();

		return false;
	}

	void GeomCompiler::MergeData(std::vector<InputMeshPart>& inputMesh)
	{
		//
		// Remove Mesh parts with zero vertices
		//
		for (auto i = 0; i < inputMesh.size(); ++i)
		{
			if (inputMesh[i].Vertices.size() == 0 || inputMesh[i].Indices.size() == 0)
			{
				inputMesh.erase(inputMesh.begin() + i);
				--i;
			}
		}

		//
		// Merge any mesh part based on Mesh and iMaterial...
		//
		for (auto i = 0; i < inputMesh.size(); ++i)
		{
			for (auto j = i + 1; j < inputMesh.size(); ++j)
			{
				// Lets find a candidate to merge...
				if (inputMesh[i].MaterialIndex == inputMesh[j].MaterialIndex
					&& inputMesh[i].MeshName == inputMesh[j].MeshName)
				{
					const int  iBaseVertex = static_cast<int>(inputMesh[i].Vertices.size());
					const auto iBaseIndex = inputMesh[i].Indices.size();
					inputMesh[i].Vertices.insert(inputMesh[i].Vertices.end(), inputMesh[j].Vertices.begin(), inputMesh[j].Vertices.end());
					inputMesh[i].Indices.insert(inputMesh[i].Indices.end(), inputMesh[j].Indices.begin(), inputMesh[j].Indices.end());

					// Fix the indices
					for (auto I = iBaseIndex; I < inputMesh[i].Indices.size(); ++I)
					{
						inputMesh[i].Indices[I] += iBaseVertex;
					}

					inputMesh.erase(inputMesh.begin() + j);
					--j;
				}
			}
		}
	}

	void GeomCompiler::Optimize(std::vector<InputMeshPart>& inputMesh)
	{
		//Vertex and index optimization
		std::vector<InputMeshPart> optimizedMeshParts;
		for (auto& meshPart : inputMesh)
		{
			std::size_t index_count = meshPart.Indices.size();
			std::vector<uint32_t> remap(index_count);
			std::size_t vertex_count = meshopt_generateVertexRemap(&remap[0], meshPart.Indices.data(), index_count, meshPart.Vertices.data(), index_count, sizeof(FullVertex));

			InputMeshPart optimized_submesh;
			optimized_submesh.Indices.resize(index_count);
			optimized_submesh.Vertices.resize(vertex_count);
			optimized_submesh.MaterialIndex = meshPart.MaterialIndex;

			//Remap indices
			meshopt_remapIndexBuffer(optimized_submesh.Indices.data(), meshPart.Indices.data(), index_count, &remap[0]);

			//Remap vertices
			meshopt_remapVertexBuffer(optimized_submesh.Vertices.data(), meshPart.Vertices.data(), index_count, sizeof(FullVertex), &remap[0]);

			//Optimize vertex cache
			meshopt_optimizeVertexCache(optimized_submesh.Indices.data(), optimized_submesh.Indices.data(), index_count, vertex_count);

			//Optimize overdraw
			meshopt_optimizeOverdraw(optimized_submesh.Indices.data(), optimized_submesh.Indices.data(), index_count, &optimized_submesh.Vertices[0].Position.x, vertex_count, sizeof(FullVertex), 1.05f);

			//Optimize vertex fetch
			optimized_submesh.Vertices.resize(meshopt_optimizeVertexFetch(optimized_submesh.Vertices.data(), optimized_submesh.Indices.data(), index_count, optimized_submesh.Vertices.data(), vertex_count, sizeof(FullVertex)));

			optimizedMeshParts.push_back(optimized_submesh);
		}

		inputMesh = std::move(optimizedMeshParts);
	}

	std::vector<GeomCompiler::CompressedMeshPart> GeomCompiler::Quantize(const std::vector<InputMeshPart>& inputMesh)
	{
		//Not gonna quantize for now
		std::vector<CompressedMeshPart> compressedMeshParts;
		compressedMeshParts.resize(inputMesh.size());

		for (auto i = 0; i < inputMesh.size(); ++i)
		{
			compressedMeshParts[i].MaterialIndex = inputMesh[i].MaterialIndex;
			compressedMeshParts[i].MeshName = inputMesh[i].MeshName;
			compressedMeshParts[i].Position.resize(inputMesh[i].Vertices.size());
			compressedMeshParts[i].Extra.resize(inputMesh[i].Vertices.size());
			compressedMeshParts[i].Indices = inputMesh[i].Indices;
			for (auto j = 0; j < inputMesh[i].Vertices.size(); ++j)
			{
				compressedMeshParts[i].Position[j].Position = inputMesh[i].Vertices[j].Position;
				compressedMeshParts[i].Extra[j].Normal = inputMesh[i].Vertices[j].fNormal;
				compressedMeshParts[i].Extra[j].UV = inputMesh[i].Vertices[j].UV;
				compressedMeshParts[i].Extra[j].Color = inputMesh[i].Vertices[j].fColor;
			}
		}

		return compressedMeshParts;
	}

	std::unique_ptr<TempGeom> GeomCompiler::CreateSkinGeom(const std::vector<CompressedMeshPart>&& compressedMesh)
	{
		// Create the final mesh
		std::unique_ptr<TempGeom> skinGeom = std::make_unique<TempGeom>();
		for (auto& E : compressedMesh)
		{
			int iFinalMesh = -1;
			for (auto i = 0; i < skinGeom->Meshes.size(); ++i)
			{
				if (skinGeom->Meshes[i].Name == E.MeshName)
				{
					iFinalMesh = i;
					break;
				}
			}

			if (iFinalMesh == -1)
			{
				iFinalMesh = static_cast<int>(skinGeom->Meshes.size());
				skinGeom->Meshes.emplace_back();
				skinGeom->Meshes.back().Name = E.MeshName;
			}


			auto& FinalMesh = skinGeom->Meshes[iFinalMesh];
			auto& SubMesh = FinalMesh.Submeshes.emplace_back();

			SubMesh.Position = E.Position;
			SubMesh.Extra = E.Extra;
			SubMesh.Indices = E.Indices;
			SubMesh.MaterialIndex = E.MaterialIndex;
			SubMesh.PosCompressionOffset = E.PosCompressionOffset;
			SubMesh.UVCompressionOffset = E.UVCompressionOffset;
		}

		return std::move(skinGeom);
	}

	void GeomCompiler::CastToGeom(std::unique_ptr<TempGeom> tempGeom)
	{
		//Get total sizes
		std::size_t totalMeshes = tempGeom->Meshes.size();
		std::size_t totalSubMeshes = 0;
		std::size_t totalVertices = 0;
		std::size_t totalExtras = 0;
		std::size_t totalIndices = 0;

		for (auto& _mesh : tempGeom->Meshes)
		{
			totalSubMeshes += _mesh.Submeshes.size();
			for (auto& _submesh : _mesh.Submeshes)
			{
				totalVertices += _submesh.Position.size();
				totalExtras += _submesh.Extra.size();
				totalIndices += _submesh.Indices.size();
			}
		}

		//Allocate memory
		auto uMesh = std::make_unique<Geom::Mesh[]>(totalMeshes);
		auto uSubMesh = std::make_unique<Geom::SubMesh[]>(totalSubMeshes);
		auto uPos = std::make_unique<Geom::Position[]>(totalVertices);
		auto uExtras = std::make_unique<Geom::Extra[]>(totalExtras);
		auto uIndices = std::make_unique<std::uint32_t[]>(totalIndices);

		auto Mesh = std::span{ uMesh.get(), static_cast<std::size_t>(totalMeshes) };
		auto SubMesh = std::span{ uSubMesh.get(), static_cast<std::size_t>(totalSubMeshes) };
		auto Pos = std::span{ uPos.get(), static_cast<std::size_t>(totalVertices) };
		auto Extras = std::span{ uExtras.get(), static_cast<std::size_t>(totalExtras) };
		auto Indices = std::span{ uIndices.get(), static_cast<std::size_t>(totalIndices) };

		m_Geom = std::make_unique<Geom>();

		//Start copying data in
		m_Geom->nMeshes = (std::uint32_t)totalMeshes;
		m_Geom->nSubMeshes = (std::uint32_t)totalSubMeshes;
		m_Geom->nPosition = (std::uint32_t)totalVertices;
		m_Geom->nExtras = (std::uint32_t)totalExtras;
		m_Geom->nIndices = (std::uint32_t)totalIndices;
		m_Geom->PosCompressionScale = tempGeom->PosCompressionScale;
		m_Geom->UVCompressionScale = tempGeom->UVCompressionScale;

		std::size_t iVertex = 0, iIndices = 0, iExtra = 0;
		for (std::size_t i = 0; i < totalMeshes; ++i)
		{
			//Copy name of mesh in
			strcpy_s(Mesh[i].Name.data(), Mesh[i].Name.size(), tempGeom->Meshes[i].Name.c_str());

			//Submesh data
			for (std::size_t j = 0; j < totalSubMeshes; ++j)
			{
				SubMesh[j].m_nFaces = (std::uint32_t)tempGeom->Meshes[i].Submeshes[j].Indices.size() / 3;
				SubMesh[j].m_iIndices = (std::uint32_t)iIndices;
				SubMesh[j].m_iVertices = (std::uint32_t)iVertex;
				SubMesh[j].m_iMaterial = (std::uint16_t)tempGeom->Meshes[i].Submeshes[j].MaterialIndex;
				SubMesh[j].m_PosCompressionOffset = tempGeom->Meshes[i].Submeshes[j].PosCompressionOffset;
				SubMesh[j].m_UVCompressionOffset = tempGeom->Meshes[i].Submeshes[j].UVCompressionOffset;

				std::size_t vertSize = tempGeom->Meshes[i].Submeshes[j].Position.size();
				std::size_t extraSize = tempGeom->Meshes[i].Submeshes[j].Extra.size();
				std::size_t indexSize = tempGeom->Meshes[i].Submeshes[j].Indices.size();

				const auto& submesh = tempGeom->Meshes[i].Submeshes[j];

				//Position
				{
					for (const auto& _pos : submesh.Position)
					{
						auto& pos = Pos[iVertex++];

						/*pos.m_QPosition_X = _pos.m_QPosition_X;
						pos.m_QPosition_Y = _pos.m_QPosition_Y;
						pos.m_QPosition_Z = _pos.m_QPosition_Z;
						pos.m_QPosition_QNormalX = _pos.m_QPosition_QNormalX;*/

						pos = _pos;
					}
				}

				//Extras
				{
					for (const auto& _extra : submesh.Extra)
					{
						auto& extra = Extras[iExtra++];

						/*extra.m_Packed = _extra.m_Packed;
						extra.m_U = _extra.m_U;
						extra.m_V = _extra.m_V;*/

						extra = _extra;
					}
				}

				//Indices
				{
					for (const auto& _indices : submesh.Indices)
					{
						auto& indice = Indices[iIndices++];

						indice = _indices;
					}
				}

				SubMesh[j].m_nVertices = (std::uint32_t)vertSize;
				SubMesh[j].m_nIndices = (std::uint32_t)indexSize;
			}
		}

		m_Geom->pMesh = uMesh.release();
		m_Geom->pSubMesh = uSubMesh.release();
		m_Geom->pPosition = uPos.release();
		m_Geom->pExtra = uExtras.release();
		m_Geom->pIndices = uIndices.release();
	}
}