#version 460
#extension GL_KHR_vulkan_glsl : enable
#extension GL_GOOGLE_include_directive : require

#include "CommonStructs.glsl"

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out flat uint outMeshId;

void main()
{
	uint index = indexAlias[INDICES_INDEX].indices[gl_BaseVertex + gl_VertexIndex];
	Vertex vertex = vertexAlias[VERTEX_INDEX].vertices[gl_VertexIndex];

	gl_Position = MVP.projection * MVP.view * MVP.model * vertex.position;
	outTexCoord = vertex.texCoord;

	outMeshId = indirectDrawAlias[INDIRECT_DRAW_INDEX].meshDraws[gl_DrawID].meshId;
}