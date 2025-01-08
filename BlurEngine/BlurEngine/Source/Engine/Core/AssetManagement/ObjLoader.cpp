#include "ObjLoader.h"
#include "Logger.h"

#include "Utility.h"

#define GLM_ENABLE_EXPERIMENTAL

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <functional>
#include <unordered_map>

namespace std
{
	template <>
	struct hash<glm::vec3>
	{
		size_t operator()(const glm::vec3& V) const
		{
			size_t Seed = 0;
			Util::HashCombine(Seed, V.x, V.y, V.z);
			return Seed;
		}
	};

	template <>
	struct hash<glm::vec2>
	{
		size_t operator()(const glm::vec2& V) const
		{
			size_t Seed = 0;
			Util::HashCombine(Seed, V.x, V.y);
			return Seed;
		}
	};

	template<>
	struct hash<EngineCore::Vertex>
	{
		size_t operator()(const EngineCore::Vertex& InVertex) const
		{
			size_t Seed = 0;
			Util::HashCombine(Seed, InVertex.Position, InVertex.Color, InVertex.Normal, InVertex.TexCoord, InVertex.Tangent, InVertex.BiTangent);
			return Seed;
		}
	};
}

namespace EngineCore
{
	
	std::shared_ptr<EngineCore::StaticMesh> OBJLoader::Load(const std::string& Filepath)
	{
		std::shared_ptr<StaticMesh> OutMesh = std::make_shared<StaticMesh>();

		tinyobj::attrib_t Attributes; // stores position, color, normal, uv, etc
		std::vector<tinyobj::shape_t> Shapes; // stores the index values for each face element
		std::vector<tinyobj::material_t> Materials; // .obj files can define specific materials per face, we currently ignore this
		std::string Warn, Err;

		if (!tinyobj::LoadObj(&Attributes, &Shapes, &Materials, &Warn, &Err, Filepath.c_str()))
		{
			BE_ERROR("{0}", Warn + Err);
		}

		Model NewModel;
		std::unordered_map<EngineCore::Vertex, uint32_t> UniqueVerticies{};

		// The load function already triangulates faces; at this point it can be assumed that there are 3 vertices on each face
		for(auto Itr = Shapes.begin(); Itr != Shapes.end(); Itr++)
		{
			for(const tinyobj::index_t Index : Itr._Ptr->mesh.indices)
			{
				Vertex NewVertex{};

				if(Index.vertex_index >= 0)
				{
					NewVertex.Position =
					{
						Attributes.vertices[3 * Index.vertex_index],
						Attributes.vertices[3 * Index.vertex_index + 1],
						Attributes.vertices[3 * Index.vertex_index + 2]
					};
				}

				if(Index.normal_index >= 0)
				{
					NewVertex.Normal =
					{
						Attributes.normals[3 * Index.normal_index],
						Attributes.normals[3 * Index.normal_index + 1],
						Attributes.normals[3 * Index.normal_index + 2]
					};
				}

				if(Index.texcoord_index >= 0)
				{
					NewVertex.TexCoord =
					{
						Attributes.texcoords[2 * Index.texcoord_index],
						Attributes.texcoords[2 * Index.texcoord_index + 1]
					};
				}

				if(UniqueVerticies.count(NewVertex) == 0)
				{
					CalculateAABB(NewModel, NewVertex);

					UniqueVerticies[NewVertex] = static_cast<uint32_t>(NewModel.Vertices.size());
					NewModel.Vertices.push_back(NewVertex);
					NewModel.Vertices16Bit.push_back(NewVertex.To16BitVertex());
				}

				NewModel.Indices.push_back(UniqueVerticies[NewVertex]);
			}
		}

		CalculateTangentBasis(NewModel);

		NewModel.Extents = (NewModel.MaxAABB - NewModel.MinAABB) * 0.5f;
		NewModel.Center = NewModel.MinAABB + NewModel.Extents;

		OutMesh->Models.push_back(NewModel);
		OutMesh->IndexCount += (uint32_t)NewModel.Indices.size();
		OutMesh->TotalVertexSize += sizeof(Vertex) * (uint32_t)OutMesh->Models.back().Vertices.size();
		OutMesh->TotalIndexSize += sizeof(uint32_t) * (uint32_t)OutMesh->Models.back().Indices.size();
		
		return OutMesh;
	}

	void OBJLoader::CalculateAABB(Model& OutModel, const Vertex& InVertex)
	{
		if (InVertex.Position.x < OutModel.MinAABB.x)
		{
			OutModel.MinAABB.x = InVertex.Position.x;
		}

		if(InVertex.Position.y < OutModel.MinAABB.y)
		{
			OutModel.MinAABB.y = InVertex.Position.y;
		}

		if(InVertex.Position.z < OutModel.MinAABB.z)
		{
			OutModel.MinAABB.z = InVertex.Position.z;
		}

		if (InVertex.Position.x > OutModel.MaxAABB.x)
		{
			OutModel.MaxAABB.x = InVertex.Position.x;
		}

		if (InVertex.Position.y < OutModel.MaxAABB.y)
		{
			OutModel.MaxAABB.y = InVertex.Position.y;
		}

		if (InVertex.Position.z < OutModel.MaxAABB.z)
		{
			OutModel.MaxAABB.z = InVertex.Position.z;
		}
	}

	void OBJLoader::CalculateTangentBasis(Model& OutModel)
	{
		for(int i = 0; i < OutModel.Indices.size(); i += 3)
		{
			Vertex& V0 = OutModel.Vertices[OutModel.Indices[i]];
			Vertex& V1 = OutModel.Vertices[OutModel.Indices[i + 1]];
			Vertex& V2 = OutModel.Vertices[OutModel.Indices[i + 2]];

			glm::vec3 Edge1 = V1.Position - V0.Position;
			glm::vec3 Edge2 = V2.Position - V0.Position;

			glm::vec2 DeltaUV1 = V1.TexCoord - V0.TexCoord;
			glm::vec2 DeltaUV2 = V2.TexCoord - V0.TexCoord;

			float Determinant = 1.0f / (DeltaUV1.x * DeltaUV2.y - DeltaUV2.x * DeltaUV1.y);

			glm::vec3 Tangent = Determinant * (DeltaUV2.y * Edge1 - DeltaUV1.y * Edge2);
			glm::vec3 BiTangent = Determinant * (DeltaUV1.x * Edge2 - DeltaUV2.x * Edge1);

			V0.Tangent += Tangent;
			V1.Tangent += Tangent;
			V2.Tangent += Tangent;

			V0.BiTangent += BiTangent;
			V1.BiTangent += BiTangent;
			V2.BiTangent += BiTangent;
		}

		for(size_t i = 0; i < OutModel.Vertices.size(); i++)
		{
			Vertex& V = OutModel.Vertices[i];

			V.Tangent = glm::normalize(V.Tangent - glm::dot(V.Tangent, V.Normal) * V.Normal);
			V.BiTangent = glm::normalize(glm::cross(V.Normal, V.Tangent));

			OutModel.Vertices16Bit[i] = OutModel.Vertices[i].To16BitVertex();
		}
	}

}