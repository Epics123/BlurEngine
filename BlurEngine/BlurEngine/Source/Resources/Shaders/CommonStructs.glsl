#ifndef COMMON_SHADER_STRUCTS
#define COMMON_SHADER_STRUCTS

struct Vertex
{
	vec4 position;
	vec4 color;
	vec4 normal;
	vec4 tangent;
	vec4 biTangent;
	vec2 texCoord;
};

struct IndirectDrawAndMeshData
{
	uint indexCount;
	uint instanceCount;
	uint firstIndex;
	uint vertexOffset;
	uint firstInstance;

	uint meshId;
};

layout(set = 0, binding = 0) uniform Matricies
{
	mat4 model;
	mat4 view;
	mat4 projection;
	mat4 prevView;
	mat4 jitter;
} 
MVP;

layout(set = 3, binding = 0) readonly buffer VertexBuffer
{
	Vertex vertices[];
}
vertexAlias[4];

layout(set = 3, binding = 0) readonly buffer IndexBuffer
{
	uint indices[];
}
indexAlias[4];

layout(set = 3, binding = 0) readonly buffer IndirectDrawAndMeshDataBuffer
{
	IndirectDrawAndMeshData meshDraws[];
}
indirectDrawAlias[4];

const int VERTEX_INDEX = 0;
const int INDICES_INDEX = 1;
const int INDIRECT_DRAW_INDEX = 2;

#endif