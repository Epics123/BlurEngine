#version 460
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec4 VertexColor;
layout(location = 0) out vec4 OutColor;

void main()
{
	OutColor = vec4(1.0, 0.0, 0.0, 1.0);//VertexColor;
}