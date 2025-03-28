#version 460
#extension GL_KHR_vulkan_glsl : enable
#extension GL_GOOGLE_include_directive : require

#include "CommonStructs.glsl"

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in flat uint inMeshId;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(0.5, 0.5, 0.5, 1.0f);
}