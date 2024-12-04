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

	void Init(GLFWkeyfun KeyCallback, GLFWcursorposfun CursorPosCallback, GLFWmousebuttonfun MouseButtonCallback,
			  GLFWscrollfun MouseScrollCallback, GLFWframebuffersizefun FramebufferResizeCallback, void* User);
	void Cleanup();

	void SetFrameBufferResized(const bool Resized) { FrameBufferResized = Resized; }

	void SetWidth(const uint32_t NewWidth) { Width = NewWidth; }
	void SetHeight(const uint32_t NewHeight) { Height = NewHeight; }

	const std::string& GetName() { return Name; }

	GLFWwindow* GetGLFWWindow() const { return GLFWWindow; }

	void CreateWindowSurface(VkInstance Instance, VkSurfaceKHR* Surface);

private:
	bool Resizeable;
	bool FrameBufferResized = false;

	uint32_t Width;
	uint32_t Height;

	std::string Name;

	GLFWwindow* GLFWWindow = nullptr;
};