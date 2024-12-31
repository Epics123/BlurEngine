#version 460
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) out vec4 OutColor;

vec2 Positions[3] = {vec2(0.0, -0.5), vec2(0.5, 0.5), vec2(-0.5, 0.5)};
vec3 Colors[3] = {vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0)};

void main()
{
	gl_Position = vec4(Positions[gl_VertexIndex], 0.0, 1.0);
	OutColor = vec4(Colors[gl_VertexIndex], 1.0);
}