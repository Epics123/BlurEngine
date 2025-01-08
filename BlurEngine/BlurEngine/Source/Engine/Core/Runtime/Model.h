#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <vector>

namespace EngineCore
{

struct Vertex16Bit
{
	glm::uint16_t PosX;
	glm::uint16_t PosY;
	glm::uint16_t PosZ;
	glm::uint16_t ColorX;
	glm::uint16_t ColorY;
	glm::uint16_t ColorZ;
	glm::uint16_t NormalX;
	glm::uint16_t NormalY;
	glm::uint16_t NormalZ;
	glm::uint16_t TangentX;
	glm::uint16_t TangentY;
	glm::uint16_t TangentZ;
	glm::uint16_t BiTangentX;
	glm::uint16_t BiTangentY;
	glm::uint16_t BiTangentZ;
	glm::uint16_t TexcoordU;
	glm::uint16_t TexcoordV;
};

struct Vertex
{
	glm::vec3 Position{};
	glm::vec3 Color{};
	glm::vec3 Normal{};
	glm::vec3 Tangent{};
	glm::vec3 BiTangent{};
	glm::vec2 TexCoord{};

	void ApplyTransform(const glm::mat4& Transform)
	{
		glm::vec4 NewPos = Transform * glm::vec4(Position, 1.0f);
		Position = glm::vec3(NewPos.x, NewPos.y, NewPos.z);

		glm::mat3 NormalMatrix = glm::inverseTranspose(glm::mat3(Transform));
		Normal = NormalMatrix * Normal;

		Tangent = glm::inverseTranspose(Transform) * glm::vec4(Tangent, 1.0f);
	}

	bool operator==(const Vertex& Other) const
	{
		return Position == Other.Position && Color == Other.Color && Normal == Other.Normal && TexCoord == Other.TexCoord;
	}

	Vertex16Bit To16BitVertex();
};

// TODO: Should probably move this to it's own file
struct Material
{
	glm::vec4 BaseColorIDs = glm::vec4(-1, -1, 0, 0); // X = texture ID, Y = sampler ID, Z & W ignored for padding
	glm::vec4 MetallicData = glm::vec4(-1, -1, 1.0f, 1.0f); // X = texture ID, Y = sampler ID, Z = metallic factor, W = roughness
	glm::vec2 NormalIDs = glm::vec2(-1, -1); // X = texture ID, Y = sampler ID
	glm::vec2 EmissiveIDs = glm::vec2(-1, -1); // X = texture ID, Y = sampler ID

	glm::vec4 BaseColor;
};

struct IndirectDrawData
{
	uint32_t IndexCount;
	uint32_t InstanceCount;
	uint32_t FirstIndex;
	uint32_t VertexOffset;
	uint32_t FirstInstance;

	uint32_t MeshID;
	int32_t MaterialIndex;

	// Can probably add things like model mat or other mesh specific things
};

struct Model
{
	std::vector<Vertex> Vertices = {};
	std::vector<Vertex16Bit> Vertices16Bit = {};
	std::vector<uint32_t> Indices = {};

	glm::vec3 MinAABB = glm::vec3(999999, 999999, 999999);
	glm::vec3 MaxAABB = glm::vec3(-999999, -999999, -999999);

	glm::vec3 Extents;
	glm::vec3 Center;

	int32_t MaterialIndex = -1;
};

struct StaticMesh
{
	std::vector<Model> Models;
	std::vector<Material> Materials;

	std::vector<IndirectDrawData> IndirectDrawDataSet;

	uint32_t TotalVertexSize = 0;
	uint32_t TotalIndexSize = 0;
	uint32_t IndexCount = 0;
};

}