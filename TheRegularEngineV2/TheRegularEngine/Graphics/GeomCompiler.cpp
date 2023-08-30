#include "pch.h"
#include "Core/Logger.h"
#include "GeomCompiler.h"
#include "meshoptimizer.h"

namespace TRE
{
	void GeomCompiler::Compile(const std::string& filename)
	{
		TRE_CORE_INFO("Importing mesh from: {0}", filename);
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

		m_Scene = importer.ReadFile(filename, flag);

		assert(m_Scene != nullptr && "Error loading model");

		if (SanityCheck())
		{
			TRE_CORE_ERROR("Sanity check failed");
			return;
		}

		ImportData();
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
		for (auto i = 0u; i < inputMesh.size(); ++i)
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
		for (auto i = 0u; i < inputMesh.size(); ++i)
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
		TRE_CORE_INFO("Optimizing mesh...");
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
}