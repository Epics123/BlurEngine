#pragma once

#define GLFW_INCLUDE_VULKAN

#include <glfw3.h>
#include <glfw3native.h>

#include <string>
#include <memory>

class Window
{
public:
	Window() = delete;
	Window(const std::string& WindowName, const uint32_t WindowWidth,  
		   const uint32_t WindowHeight, const bool CanResize = false);

	void Init(GLFWkeyfun keyCallback, GLFWcursorposfun cursorPosCallback, GLFWmousebuttonfun mouseButtonCallback,
			  GLFWscrollfun mouseScrollCallback, GLFWframebuffersizefun framebufferResizeCallback, void* user);
	void Cleanup();

private:
	bool Resizeable;
	uint32_t Width;
	uint32_t Height;

	std::string Name;

	GLFWwindow* GLFWWindow = nullptr;
};