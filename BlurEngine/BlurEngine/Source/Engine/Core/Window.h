#pragma once

#define GLFW_INCLUDE_VULKAN

#include <glfw3.h>
#include <glfw3native.h>

class Window
{
public:
	Window() = delete;
	Window(const std::string& WindowName, const uint32_t WindowWidth,  const uint32_t WindowHeight, const bool CanResize = false)
};