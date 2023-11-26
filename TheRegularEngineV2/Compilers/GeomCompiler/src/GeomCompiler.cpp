#include "GeomCompiler.h"
#include "meshoptimizer.h"
#include <span>
#include <filesystem>
#include <functional>
#include <fstream>
#include <sstream>
#include <iostream>

namespace TRE
{
	void GeomCompiler::Compile(const GeomDescriptorFile& geomDesc)
	{
		Assimp::Importer importer;

		uint32_t flag = 
			  aiProcess_Triangulate                // Make sure we get triangles rather than nvert polygons
			| aiProcess_LimitBoneWeights           // 4 weights for skin model max
			| aiProcess_GenUVCoords                // Convert any type of mapping to uv mapping
			| aiProcess_TransformUVCoords          // preprocess UV transformations (scaling, translation ...)
			| aiProcess_FindInstances              // search for instanced meshes and remove them by references to one master
			| aiProcess_CalcTangentSpace           // calculate tangents and bitangents if possible (definetly you will meed UVs)
			| aiProcess_RemoveRedundantMaterials   // remove redundant materials
			| aiProcess_FindInvalidData            // detect invalid model data, such as invalid normal vectors
			| aiProcess_FlipUVs                    // flip the V to match the Vulkans way of doing UVs
			;

		const std::string& filePath = geomDesc.GetAssetPath();
		m_filePath = filePath;
		m_Scene = importer.ReadFile(filePath, flag);
		if (m_Scene == nullptr)
		{
			std::cout << "Error loading model: " << filePath << std::endl;
			return;
		}
		//assert(m_Scene != nullptr && "Error loading model");

		if (SanityCheck())
		{
			//TRE_CORE_ERROR("Sanity check failed");
			return;
		}

		ImportGeometry(geomDesc);
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
				//TRE_CORE_WARN("Mesh with no reference in scene");
				return true;
			}

			if (AssimpMesh.HasBones())
			{
				if (Refs.size() > 1)
				{
					//TRE_CORE_WARN("Found a skin mesh with multiple references in scene, Unsupported");
					return true;
				}
			}
			else
			{
				if (Refs.size() > 1)
				{
					//TRE_CORE_INFO("Mesh with multiple references in scene, Duplicate");
				}
			}
		}

		return false;
	}

	void GeomCompiler::ImportGeometry(const GeomDescriptorFile& geomDesc)
	{
		std::vector<InputMeshPart> MyNodes;
		
		ImportSkeleton();
		if (m_Skeleton.m_Bones.size()) //Has animation
		{
			m_IsAnimatable = true;
			ImportAnimations();
			ImportGeometrySkin(MyNodes);
			std::cout << "Geometry has animation" << std::endl;
		}
		else
		{
			m_IsAnimatable = false;
			ImportStaticMesh(MyNodes, geomDesc);
			std::cout << "Geometry is static mesh" << std::endl;
		}

		MergeData(MyNodes);

		if(geomDesc.GetOptimize() && !m_IsAnimatable)
			Optimize(MyNodes);

		auto skinGeom = CreateSkinGeom(Quantize(MyNodes));

		CastToGeom(std::move(skinGeom));
	}

	void GeomCompiler::ImportStaticMesh(std::vector<InputMeshPart>& inputMesh, const GeomDescriptorFile& geomDesc)
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

					if (AssimpMesh.HasNormals() == false)
						std::cout << "Model contains no normals" << std::endl;
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

		aiMatrix4x4 L2W = aiMatrix4x4();
		aiVector3D scaling((ai_real)geomDesc.GetScale().x, (ai_real)geomDesc.GetScale().y, (ai_real)geomDesc.GetScale().z);
		aiVector3D translation((ai_real)geomDesc.GetPosition().x, (ai_real)geomDesc.GetPosition().y, (ai_real)geomDesc.GetPosition().z);
		aiMatrix4x4 scale;
		aiMatrix4x4::Scaling(scaling, scale);
		aiMatrix4x4 rotationX;
		aiMatrix4x4::RotationX((ai_real)geomDesc.GetRotation().x, rotationX);
		aiMatrix4x4 rotationY;
		aiMatrix4x4::RotationY((ai_real)geomDesc.GetRotation().y, rotationY);
		aiMatrix4x4 rotationZ;
		aiMatrix4x4::RotationZ((ai_real)geomDesc.GetRotation().z, rotationZ);
		aiMatrix4x4 translationMatrix;
		aiMatrix4x4::Translation(translation, translationMatrix);
		L2W = scale * rotationX * rotationY * rotationZ * translationMatrix;

		RecurseScene(*m_Scene->mRootNode, L2W);

		std::cout << "Number of Meshes: " << m_Scene->mNumMeshes << std::endl;
	}

	bool GeomCompiler::ImportGeometryValidateMesh(const aiMesh& AssimpMesh, int& iTexture, int& iColor)
	{
		if (AssimpMesh.HasPositions() == false)
		{
			//TRE_CORE_INFO("Removing mesh ({0}) without position", AssimpMesh.mName.C_Str());
			return true;
		}

		if (AssimpMesh.HasFaces() == false)
		{
			//TRE_CORE_INFO("Removing mesh ({0}) without position", AssimpMesh.mName.C_Str());
			return true;
		}

		if (AssimpMesh.HasNormals() == false)
		{
			//TRE_CORE_INFO("Removing mesh ({0}) without normals", AssimpMesh.mName.C_Str());
			return true;
		}

		if (AssimpMesh.HasTangentsAndBitangents() == false)
		{
			//TRE_CORE_INFO("Creating tangents and bitangents for mesh ({0})", AssimpMesh.mName.C_Str());
		}

		if (AssimpMesh.GetNumUVChannels() != 1)
		{
			if (AssimpMesh.GetNumUVChannels() == 0)
			{
				//TRE_CORE_INFO("No UVs found for mesh ({0})", AssimpMesh.mName.C_Str());
			}
			else
			{
				//TRE_CORE_INFO("Too many UV channels found for mesh ({0})", AssimpMesh.mName.C_Str());
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
		// Remove Mesh parts with zero vertices
		for (auto i = 0; i < inputMesh.size(); ++i)
		{
			if (inputMesh[i].Vertices.size() == 0 || inputMesh[i].Indices.size() == 0)
			{
				inputMesh.erase(inputMesh.begin() + i);
				--i;
			}
		}

		// Merge any mesh part based on Mesh and iMaterial
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
			optimized_submesh.m_BoneInfluence = meshPart.m_BoneInfluence;

			//Remap indices
			meshopt_remapIndexBuffer(optimized_submesh.Indices.data(), meshPart.Indices.data(), index_count, &remap[0]);

			//Remap vertices
			meshopt_remapVertexBuffer(optimized_submesh.Vertices.data(), meshPart.Vertices.data(), index_count, sizeof(FullVertex), &remap[0]);

			//Optimize vertex cache
			meshopt_optimizeVertexCache(optimized_submesh.Indices.data(), optimized_submesh.Indices.data(), index_count, vertex_count);

			//Optimize overdraw
			meshopt_optimizeOverdraw(optimized_submesh.Indices.data(), optimized_submesh.Indices.data(), index_count, &optimized_submesh.Vertices[0].Position.x, vertex_count, sizeof(FullVertex), 1.03f);

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
			compressedMeshParts[i].m_BoneInfluence = inputMesh[i].m_BoneInfluence;

			for (auto j = 0; j < inputMesh[i].Vertices.size(); ++j)
			{
				compressedMeshParts[i].Position[j].Position = inputMesh[i].Vertices[j].Position;
				compressedMeshParts[i].Extra[j].Normal = inputMesh[i].Vertices[j].fNormal;
				compressedMeshParts[i].Extra[j].Tangent = inputMesh[i].Vertices[j].fTangent;
				compressedMeshParts[i].Extra[j].Bitangent = inputMesh[i].Vertices[j].fBitangent;
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
			if (m_IsAnimatable)
				SubMesh.Bone = E.m_BoneInfluence;
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
		std::size_t totalBones = 0;

		for (auto& _mesh : tempGeom->Meshes)
		{
			totalSubMeshes += _mesh.Submeshes.size();
			for (auto& _submesh : _mesh.Submeshes)
			{
				totalVertices += _submesh.Position.size();
				totalExtras += _submesh.Extra.size();
				totalIndices += _submesh.Indices.size();
				if (m_IsAnimatable)
					totalBones += _submesh.Bone.size();
			}
		}

		//Allocate memory
		auto uMesh = std::make_unique<Geom::Mesh[]>(totalMeshes);
		auto uSubMesh = std::make_unique<Geom::SubMesh[]>(totalSubMeshes);
		auto uPos = std::make_unique<Geom::Position[]>(totalVertices);
		auto uExtras = std::make_unique<Geom::Extra[]>(totalExtras);
		auto uIndices = std::make_unique<std::uint32_t[]>(totalIndices);
		auto uBones = std::make_unique<BoneInfluence[]>(totalBones);

		auto Mesh = std::span{ uMesh.get(), static_cast<std::size_t>(totalMeshes) };
		auto SubMesh = std::span{ uSubMesh.get(), static_cast<std::size_t>(totalSubMeshes) };
		auto Pos = std::span{ uPos.get(), static_cast<std::size_t>(totalVertices) };
		auto Extras = std::span{ uExtras.get(), static_cast<std::size_t>(totalExtras) };
		auto Indices = std::span{ uIndices.get(), static_cast<std::size_t>(totalIndices) };
		auto Bones = std::span{ uBones.get(), static_cast<std::size_t>(totalBones) };

		m_Geom = std::make_unique<Geom>();

		//Start copying data in
		m_Geom->nMeshes = (std::uint32_t)totalMeshes;
		m_Geom->nSubMeshes = (std::uint32_t)totalSubMeshes;
		m_Geom->nPosition = (std::uint32_t)totalVertices;
		m_Geom->nExtras = (std::uint32_t)totalExtras;
		m_Geom->nIndices = (std::uint32_t)totalIndices;
		if (m_IsAnimatable)
			m_Geom->nBones = (std::uint32_t)totalBones;
		m_Geom->PosCompressionScale = tempGeom->PosCompressionScale;
		m_Geom->UVCompressionScale = tempGeom->UVCompressionScale;
		m_Geom->m_IsAnimated = m_IsAnimatable;

		std::size_t iVertex = 0, iIndices = 0, iExtra = 0, iBones = 0;
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
				if (m_IsAnimatable)
					SubMesh[j].m_iBones = (std::uint32_t)iBones;

				std::size_t vertSize = tempGeom->Meshes[i].Submeshes[j].Position.size();
				std::size_t extraSize = tempGeom->Meshes[i].Submeshes[j].Extra.size();
				std::size_t indexSize = tempGeom->Meshes[i].Submeshes[j].Indices.size();
				std::size_t BoneSize = tempGeom->Meshes[i].Submeshes[j].Bone.size();
				(void)extraSize;

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

				//Bones
				if (m_IsAnimatable)
				{
					for (const auto& _Bone : submesh.Bone)
					{
						auto& bone = Bones[iBones++];
						bone = _Bone;
					}
				}

				SubMesh[j].m_nVertices = (std::uint32_t)vertSize;
				SubMesh[j].m_nIndices = (std::uint32_t)indexSize;
				if (m_IsAnimatable)
					SubMesh[j].m_nBones = (std::uint32_t)BoneSize;
			}
		}

		m_Geom->pMesh = uMesh.release();
		m_Geom->pSubMesh = uSubMesh.release();
		m_Geom->pPosition = uPos.release();
		m_Geom->pExtra = uExtras.release();
		m_Geom->pIndices = uIndices.release();
		if (m_IsAnimatable)
			m_Geom->pBone = uBones.release();
	}

	//For animation
	void GeomCompiler::ImportSkeleton()
	{
		std::unordered_map<std::string, const aiNode*> NameToNode;
		std::unordered_map<std::string, const aiBone*> NameToBone;

		// Add bones base on bone associated by meshes
		for (auto iMesh = 0u; iMesh < m_Scene->mNumMeshes; ++iMesh)
		{
			const aiMesh& Mesh = *m_Scene->mMeshes[iMesh];
			for (auto iBone = 0u; iBone < Mesh.mNumBones; ++iBone)
			{
				const aiBone& Bone = *Mesh.mBones[iBone];
				if (auto E = NameToBone.find(Bone.mName.data); E == NameToBone.end())
				{
					auto pNode = m_Scene->mRootNode->FindNode(Bone.mName);
					NameToBone[Bone.mName.data] = &Bone;
					NameToNode[Bone.mName.data] = pNode;
				}
			}
		}

		// Make sure all the parent nodes are inserted in the hash table
		// This algotithum is a bit overkill but is ok
		for (auto itr1 : NameToNode)
		{
			for (auto pParentNode = NameToNode.find(itr1.first)->second->mParent; pParentNode != nullptr; pParentNode = pParentNode->mParent)
			{
				if (auto e = NameToNode.find(pParentNode->mName.C_Str()); e == NameToNode.end())
				{
					NameToNode[pParentNode->mName.C_Str()] = pParentNode;
				}
			}
		}

		// Check to see if we readed too many bones!
		if (NameToNode.size() > 0xff)
		{
			std::cout << "ERROR: This mesh has {0} Bones we can only handle up to 256" << std::endl;
		}

		// Organize build the skeleton 
		// We want the parents to be first then the children
		// Ideally we also want to have the bones that have more children higher
		struct proto
		{
			const aiNode* m_pAssimpNode{ nullptr };
			int             m_Depth{ 0 };
			int             m_nTotalChildren{ 0 };
			int             m_nChildren{ 0 };
		};
		std::vector<proto> Proto;

		// Set the Assimp Node
		Proto.resize(NameToNode.size());
		{
			int i = 0;
			for (auto itr = NameToNode.begin(); itr != NameToNode.end(); ++itr)
			{
				auto& P = Proto[i++];
				P.m_pAssimpNode = itr->second;
			}
		}

		// Set the Depth, m_nTotalChildren and nChildren
		for (auto i = 0u; i < Proto.size(); ++i)
		{
			auto& P = Proto[i];
			bool  bFoundParent = false;

			for (aiNode* pNode = P.m_pAssimpNode->mParent; pNode; pNode = pNode->mParent)
			{
				P.m_Depth++;

				// If we can find the parent lets keep a count of how many total children it has
				for (auto j = 0; j < Proto.size(); ++j)
				{
					auto& ParentProto = Proto[j];
					if (pNode == ParentProto.m_pAssimpNode)
					{
						ParentProto.m_nTotalChildren++;
						if (bFoundParent == false) ParentProto.m_nChildren++;
						bFoundParent = true;
						break;
					}
				}
			}
		}

		// Put all the Proto bones in the right order
		std::qsort(Proto.data(), Proto.size(), sizeof(proto), [](const void* pA, const void* pB) -> int
			{
				const auto& A = *reinterpret_cast<const proto*>(pA);
				const auto& B = *reinterpret_cast<const proto*>(pB);

				if (A.m_Depth < B.m_Depth) return -1;
				if (A.m_Depth > B.m_Depth) return  1;
				if (A.m_nTotalChildren < B.m_nTotalChildren) return  -1;
				return (A.m_nTotalChildren > B.m_nTotalChildren);
			});

		// Create all the real bones
		m_Skeleton.m_Bones.resize(Proto.size());
		{
			int i = 0;
			for (auto& ACBone : m_Skeleton.m_Bones)
			{
				auto& ProtoBone = Proto[i++];
				ACBone.m_Name = ProtoBone.m_pAssimpNode->mName.data;
				ACBone.m_iParent = -1;

				// Potentially we may not have all parent nodes in our skeleton
				// so we must search by each of the potential assimp nodes
				for (aiNode* pNode = ProtoBone.m_pAssimpNode->mParent; ACBone.m_iParent == -1 && pNode; pNode = pNode->mParent)
				{
					for (auto j = 0; j < i; ++j)
					{
						if (Proto[j].m_pAssimpNode == ProtoBone.m_pAssimpNode->mParent)
						{
							ACBone.m_iParent = j;
							break;
						}
					}
				}

				// Check if we have a binding matrix
				if (auto B = NameToBone.find(ProtoBone.m_pAssimpNode->mName.data); B != NameToBone.end())
				{
					// Inverse bind matrix
					auto OffsetMatrix = B->second->mOffsetMatrix;
					auto NodeMatrix = m_References[0].Nodes[0]->mTransformation;
					NodeMatrix = OffsetMatrix * NodeMatrix.Inverse();

					std::memcpy(&ACBone.m_InvBind, &NodeMatrix, sizeof(glm::mat4));
					ACBone.m_InvBind = glm::transpose(ACBone.m_InvBind);
				}
				else
				{
					ACBone.m_InvBind = glm::identity<glm::mat4>();
				}

				// Neutral pose
				{
					const auto  C = NameToNode.find(ProtoBone.m_pAssimpNode->mName.data);
					auto        NodeMatrix = C->second->mTransformation;
					for (auto p = C->second->mParent; p; p = p->mParent) NodeMatrix = p->mTransformation * NodeMatrix;
					std::memcpy(&ACBone.m_NeutalPose, &NodeMatrix, sizeof(glm::mat4));
					ACBone.m_NeutalPose = glm::transpose(ACBone.m_NeutalPose);

					ACBone.m_NeutalPose *= ACBone.m_InvBind;
				}

			}
		}
	}

	void GeomCompiler::ImportAnimations()
	{
		int SamplingFPS = 60;
		struct indices
		{
			std::uint32_t m_iPositions{ 0 };
			std::uint32_t m_iRotations{ 0 };
			std::uint32_t m_iScales   { 0 };
		};

		m_AnimPackage.m_Animations.resize(m_Scene->mNumAnimations);
		for (auto i = 0ul; i < m_Scene->mNumAnimations; ++i)
		{
			const aiAnimation& AssimpAnim = *m_Scene->mAnimations[i];
			const double            AnimationDuration = AssimpAnim.mDuration / AssimpAnim.mTicksPerSecond;
			const double            DeltaTime = (AssimpAnim.mTicksPerSecond / SamplingFPS);
			const int               FrameCount = (int)std::ceil(AssimpAnim.mDuration / DeltaTime);
			assert(FrameCount > 0);
			std::vector<indices>    LastPositions;

			// Allocate all the bones for this animation
			// assert( AssimpAnim.mNumChannels <= m_pAnimCharacter->m_Skeleton.m_Bones.size() );
			auto& MyAnim = m_AnimPackage.m_Animations[i];
			MyAnim.m_BoneKeyFrames.resize(m_Skeleton.m_Bones.size());
			MyAnim.m_FPS = SamplingFPS;
			MyAnim.m_Name = AssimpAnim.mName.C_Str();
			MyAnim.m_TimeLength = static_cast<float>(AnimationDuration);

			// To cache the last positions for a given frame for each bone
			LastPositions.resize(AssimpAnim.mNumChannels);

			// Create/Sample all the frames            
			for (int iFrame = 0; iFrame < FrameCount; iFrame++)
			{
				const auto t = iFrame * DeltaTime;
				for (auto b = 0ul; b < AssimpAnim.mNumChannels; ++b)
				{
					const aiNodeAnim& Channel = *AssimpAnim.mChannels[b];
					auto& LastPos = LastPositions[b];

					// Sample the position key
					aiVector3D presentPosition(0, 0, 0);
					if (Channel.mNumPositionKeys > 0)
					{
						// Update the Position Index for the given bone
						while (LastPos.m_iPositions < Channel.mNumPositionKeys - 1)
						{
							if (t < Channel.mPositionKeys[LastPos.m_iPositions + 1].mTime) break;
							++LastPos.m_iPositions;
						}

						// interpolate between this frame's value and next frame's value
						unsigned int        NextFrame = (LastPos.m_iPositions + 1) % Channel.mNumPositionKeys;
						const aiVectorKey& Key = Channel.mPositionKeys[LastPos.m_iPositions];
						const aiVectorKey& NextKey = Channel.mPositionKeys[NextFrame];
						double              diffTime = NextKey.mTime - Key.mTime;

						if (diffTime < 0.0) diffTime += AssimpAnim.mDuration;
						if (diffTime > 0)
						{
							float factor = float((t - Key.mTime) / diffTime);
							presentPosition = Key.mValue + (NextKey.mValue - Key.mValue) * factor;
						}
						else
						{
							presentPosition = Key.mValue;
						}
					}

					// Sample the Rotation key
					aiQuaternion presentRotation(1, 0, 0, 0);
					if (Channel.mNumRotationKeys > 0)
					{
						// Update the Rotation Index for the given bone
						while (LastPos.m_iRotations < Channel.mNumRotationKeys - 1)
						{
							if (t < Channel.mRotationKeys[LastPos.m_iRotations + 1].mTime) break;
							++LastPos.m_iRotations;
						}

						// interpolate between this frame's value and next frame's value
						unsigned int        NextFrame = (LastPos.m_iRotations + 1) % Channel.mNumRotationKeys;
						const aiQuatKey& Key = Channel.mRotationKeys[LastPos.m_iRotations];
						const aiQuatKey& NextKey = Channel.mRotationKeys[NextFrame];
						double              diffTime = NextKey.mTime - Key.mTime;

						if (diffTime < 0.0) diffTime += AssimpAnim.mDuration;
						if (diffTime > 0)
						{
							float factor = float((t - Key.mTime) / diffTime);
							aiQuaternion::Interpolate(presentRotation, Key.mValue, NextKey.mValue, factor);
						}
						else
						{
							presentRotation = Key.mValue;
						}
					}

					// Sample the Scale key
					aiVector3D presentScaling(1, 1, 1);
					if (Channel.mNumScalingKeys > 0)
					{
						// Update the Rotation Index for the given bone
						while (LastPos.m_iScales < Channel.mNumScalingKeys - 1)
						{
							if (t < Channel.mScalingKeys[LastPos.m_iScales + 1].mTime) break;
							++LastPos.m_iScales;
						}

						// TODO: interpolation maybe? This time maybe even logarithmic, not linear!
						// interpolate between this frame's value and next frame's value
						unsigned int        NextFrame = (LastPos.m_iScales + 1) % Channel.mNumScalingKeys;
						const aiVectorKey& Key = Channel.mScalingKeys[LastPos.m_iScales];
						const aiVectorKey& NextKey = Channel.mScalingKeys[NextFrame];
						double              diffTime = NextKey.mTime - Key.mTime;

						if (diffTime < 0.0) diffTime += AssimpAnim.mDuration;
						if (diffTime > 0)
						{
							float factor = float((t - Key.mTime) / diffTime);
							presentScaling = Key.mValue + (NextKey.mValue - Key.mValue) * factor;
						}
						else
						{
							presentScaling = Key.mValue;
						}
					}

					// Set all the computer components into our frame
					// make sure that we can find the bone                         
					const int iBone = m_Skeleton.findBone(Channel.mNodeName.C_Str());
					if (-1 == iBone)
					{
						continue;
					}

					if (MyAnim.m_BoneKeyFrames[iBone].m_Scale.size() == 0)
					{
						MyAnim.m_BoneKeyFrames[iBone].m_Scale.resize(FrameCount);
						MyAnim.m_BoneKeyFrames[iBone].m_Rotate.resize(FrameCount);
						MyAnim.m_BoneKeyFrames[iBone].m_Translate.resize(FrameCount);
					}

					auto& MyBoneKeyFrame = MyAnim.m_BoneKeyFrames[iBone];

					MyBoneKeyFrame.m_Translate[iFrame] = glm::vec3(presentPosition.x, presentPosition.y, presentPosition.z);
					MyBoneKeyFrame.m_Rotate[iFrame] = glm::quat(presentRotation.w, presentRotation.x, presentRotation.y, presentRotation.z);
					MyBoneKeyFrame.m_Scale[iFrame] = glm::vec3(presentScaling.x, presentScaling.y, presentScaling.z);
				}
			}

			// Add transforms without animations
			for (int j = 0; j < m_Skeleton.m_Bones.size(); ++j)
			{
				if (MyAnim.m_BoneKeyFrames[j].m_Scale.size() == 0)
				{
					MyAnim.m_BoneKeyFrames[j].m_Scale.resize(FrameCount);
					MyAnim.m_BoneKeyFrames[j].m_Rotate.resize(FrameCount);
					MyAnim.m_BoneKeyFrames[j].m_Translate.resize(FrameCount);
					auto pNode = m_Scene->mRootNode->FindNode(m_Skeleton.m_Bones[j].m_Name.c_str());

					aiQuaternion Q(0, 0, 0, 1);
					aiVector3D   S(1, 1, 1);
					aiVector3D   T(0, 0, 0);
					pNode->mTransformation.Decompose(S, Q, T);

					for (int f = 0; f < FrameCount; ++f)
					{
						auto& MyBoneKeyFrame = MyAnim.m_BoneKeyFrames[j];

						MyBoneKeyFrame.m_Translate[f] = glm::vec3(T.x, T.y, T.z);
						MyBoneKeyFrame.m_Rotate[f] = glm::quat(Q.w, Q.x, Q.y, Q.z);
						MyBoneKeyFrame.m_Scale[f] = glm::vec3(S.x, S.y, S.z);
					}
				}
			}
		}
	}

	template <typename T_CHAR> constexpr int ToCharUpper(const T_CHAR C) noexcept
	{
		return((C >= T_CHAR{'a'}) && (C <= T_CHAR{'z'})) ? C + (T_CHAR{ 'A' } - T_CHAR{'a'}) : C;
	}

	template< typename T1, typename T2 >
	static int FindStrI(const T1& Where, const T2& What)
	{
		int i = 0;
		while (Where[i])
		{
			if (ToCharUpper(Where[i]), ToCharUpper(What[0]))
			{
				int j = 1;
				while (What[j])
				{
					if (ToCharUpper(Where[i + j]) != ToCharUpper(What[j])) goto continue_loop;
					j++;
				}
				return i;
			continue_loop:;
			}
			i++;
		}
		return -1;
	}

	static std::string GetMeshNameFromNode(const aiNode& Node)
	{
		for (auto pNode = &Node; pNode; pNode = pNode->mParent)
		{
			// Using the naming convention to group meshes...
			if (FindStrI(pNode->mName.C_Str(), "MESH_") != -1)
			{
				return pNode->mName.C_Str();
			}
		}

		return {};
	}

	void GeomCompiler::ImportGeometrySkin(std::vector<InputMeshPart>& MyNodes)
	{
		// Add bones base on bone associated by meshes
		MyNodes.resize(m_Scene->mNumMeshes);
		for (auto iMesh = 0u; iMesh < m_Scene->mNumMeshes; ++iMesh)
		{
			const aiMesh& AssimpMesh = *m_Scene->mMeshes[iMesh];

			int iTexCordinates; int iColors;
			if (ImportGeometryValidateMesh(AssimpMesh, iTexCordinates, iColors)) continue;

			// Copy mesh name and Material Index
			MyNodes[iMesh].MeshName = AssimpMesh.mName.C_Str();
			MyNodes[iMesh].MaterialIndex = AssimpMesh.mMaterialIndex;

			// get the rotation for the normals
			aiQuaternion presentRotation;
			{
				aiVector3D p;
				m_References[iMesh].Nodes[0]->mTransformation.DecomposeNoScaling(presentRotation, p);
			}

			// Copy Vertices
			MyNodes[iMesh].Vertices.resize(AssimpMesh.mNumVertices);
			MyNodes[iMesh].m_BoneInfluence.resize(AssimpMesh.mNumVertices);
			for (auto i = 0u; i < AssimpMesh.mNumVertices; ++i)
			{
				FullVertex& Vertex = MyNodes[iMesh].Vertices[i];
				BoneInfluence& BoneInfo = MyNodes[iMesh].m_BoneInfluence[i];

				auto L = AssimpMesh.mVertices[i];
				L = m_References[iMesh].Nodes[0]->mTransformation * L;

				Vertex.Position = glm::vec3(static_cast<float>(L.x), static_cast<float>(L.y), static_cast<float>(L.z));

				if (iTexCordinates == -1)
				{
					Vertex.UV = glm::vec2(0, 0);
				}
				else
				{
					Vertex.UV = glm::vec2(static_cast<float>(AssimpMesh.mTextureCoords[iTexCordinates][i].x), static_cast<float>(AssimpMesh.mTextureCoords[iTexCordinates][i].y));
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

					Vertex.fTangent.x = T.x;
					Vertex.fTangent.y = T.y;
					Vertex.fTangent.z = T.z;

					assert(AssimpMesh.HasNormals());
					Vertex.fTangent.x = N.x;
					Vertex.fTangent.y = N.y;
					Vertex.fTangent.z = N.z;
					
					Vertex.fBitangent = glm::vec3(B.x, B.y, B.z);
					Vertex.fBitangent = glm::normalize(Vertex.fBitangent);

					Vertex.fNormal = glm::vec3(N.x, N.y, N.z);
					Vertex.fNormal = glm::normalize(Vertex.fNormal);
				}

				// Mark the weights as uninitialized we will be setting them later
				BoneInfo.m_BoneIndex.x = BoneInfo.m_BoneIndex.y = BoneInfo.m_BoneIndex.z = BoneInfo.m_BoneIndex.w = 0;
				BoneInfo.m_BoneWeights.x = BoneInfo.m_BoneWeights.y = BoneInfo.m_BoneWeights.z = BoneInfo.m_BoneWeights.w = 0;
			}

			// Copy the indices
			for (auto i = 0u; i < AssimpMesh.mNumFaces; ++i)
			{
				const auto& Face = AssimpMesh.mFaces[i];
				for (auto j = 0u; j < Face.mNumIndices; ++j)
					MyNodes[iMesh].Indices.push_back(Face.mIndices[j]);
			}

			// Add the bone weights
			if (AssimpMesh.mNumBones > 0)
			{
				struct tmp_weight
				{
					std::uint8_t m_iBone;
					float        m_Weight{ 0 };
				};

				struct my_weights
				{
					int                         m_Count{ 0 };
					std::array<tmp_weight, 4>    m_Weights;
				};

				std::vector<my_weights> MyWeights;
				MyWeights.resize(AssimpMesh.mNumVertices);

				// Collect bones indices and weights
				assert(m_References[iMesh].Nodes.size() == 1);

				MyNodes[iMesh].MeshName = GetMeshNameFromNode(*m_References[iMesh].Nodes[0]);
				for (auto iBone = 0u; iBone < AssimpMesh.mNumBones; iBone++)
				{
					const auto& AssimpBone = *AssimpMesh.mBones[iBone];
					const std::uint8_t iSkeletonBone = (uint8_t)m_Skeleton.findBone(AssimpBone.mName.C_Str());
					assert(m_Skeleton.findBone(AssimpBone.mName.C_Str()) != -1);

					for (auto iWeight = 0u; iWeight < AssimpBone.mNumWeights; ++iWeight)
					{
						const auto& AssimpWeight = AssimpBone.mWeights[iWeight];
						auto& MyWeight = MyWeights[AssimpWeight.mVertexId];

						MyWeight.m_Weights[MyWeight.m_Count].m_iBone = iSkeletonBone;
						MyWeight.m_Weights[MyWeight.m_Count].m_Weight = AssimpWeight.mWeight;
						// get ready for the next one
						MyWeight.m_Count++;
					}
				}

				// Sort weights, normalize and set to the final vert
				for (int iVertex = 0u; iVertex < MyWeights.size(); ++iVertex)
				{
					auto& E = MyWeights[iVertex];

					// Short from bigger to smaller
					std::qsort(E.m_Weights.data(), E.m_Weights.size(), sizeof(tmp_weight), [](const void* pA, const void* pB) ->int
						{
							auto& A = *reinterpret_cast<const tmp_weight*>(pA);
							auto& B = *reinterpret_cast<const tmp_weight*>(pB);
							if (B.m_Weight < A.m_Weight) return -1;
							return B.m_Weight > A.m_Weight;
						});

					assert(E.m_Weights[0].m_Weight >= E.m_Weights[1].m_Weight);

					// Normalize the weights
					float Total = 0;
					for (int i = 0; i < E.m_Count; ++i)
					{
						Total += E.m_Weights[i].m_Weight;
					}

					for (int i = 0; i < E.m_Count; ++i)
					{
						E.m_Weights[i].m_Weight /= Total;
					}

					// Copy Weight To the Vert
					auto& B = MyNodes[iMesh].m_BoneInfluence[iVertex];
					for (int i = 0; i < E.m_Count; ++i)
					{
						const auto& BW = E.m_Weights[i];

						switch (i)
						{
						case 0:
							B.m_BoneIndex.x = BW.m_iBone;
							B.m_BoneWeights.x = BW.m_Weight;
							break;
						case 1: B.m_BoneIndex.y = BW.m_iBone;
							B.m_BoneWeights.y = BW.m_Weight;
							break;
						case 2: B.m_BoneIndex.z = BW.m_iBone;
							B.m_BoneWeights.z = BW.m_Weight;
							break;
						case 3: B.m_BoneIndex.w = BW.m_iBone;
							B.m_BoneWeights.w = BW.m_Weight;
							break;
						}
					}
				}

				// Sanity check (make sure that all the vertices have bone and weights
				//for (auto& V : MyNodes[iMesh].m_BoneInfluence)
				//{
				//	assert(V.m_BoneWeights.x > 0);
				//}
			}
			else
			{
				// Set the weights and duplicate mesh if needed

				// Remember where was the base
				int iBase = static_cast<int>(m_References[iMesh].Nodes.size());

				// Grow the total number of meshes if we have to...
				if (iBase > 1) MyNodes.resize(MyNodes.size() + m_References[iMesh].Nodes.size() - 1);

				auto pMyNode = &MyNodes[iMesh];
				for (const auto pN : m_References[iMesh].Nodes)
				{
					pMyNode->MeshName = GetMeshNameFromNode(*pN);
					const std::uint8_t iSkeletonBone = (uint8_t)m_Skeleton.findBone(pN->mName.C_Str());
					for (auto iVertex = 0u; iVertex < AssimpMesh.mNumVertices; ++iVertex)
					{
						auto& V = pMyNode->m_BoneInfluence[iVertex];
						V.m_BoneIndex.x = iSkeletonBone;
						V.m_BoneWeights.x = 1.f;
					}

					if (iBase < MyNodes.size())
					{
						pMyNode = &MyNodes[iBase++];

						// Deep copy the mesh...
						*pMyNode = MyNodes[iMesh];
					}
				}
			}
		}
	}
}