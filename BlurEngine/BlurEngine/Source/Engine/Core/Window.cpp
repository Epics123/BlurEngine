#include "Window.h"
#include "../VulkanCore/VulkanCommon.h"

Window::Window(const std::string& WindowName, const uint32_t WindowWidth, const uint32_t WindowHeight, const bool CanResize)
	:Name{WindowName}, Width{WindowWidth}, Height{WindowHeight}, Resizeable{CanResize}
{
	
}

void Window::Init(GLFWkeyfun KeyCallback, GLFWcursorposfun CursorPosCallback, GLFWmousebuttonfun MouseButtonCallback, GLFWscrollfun MouseScrollCallback, GLFWframebuffersizefun FramebufferResizeCallback, void* User)
{
	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, Resizeable);

	GLFWWindow = glfwCreateWindow(Width, Height, Name.c_str(), nullptr, nullptr);

	glfwSetKeyCallback(GLFWWindow, KeyCallback);
	glfwSetCursorPosCallback(GLFWWindow, CursorPosCallback);
	glfwSetMouseButtonCallback(GLFWWindow, MouseButtonCallback);
	glfwSetScrollCallback(GLFWWindow, MouseScrollCallback);

	glfwSetWindowUserPointer(GLFWWindow, this);
	glfwSetFramebufferSizeCallback(GLFWWindow, FramebufferResizeCallback);
}

void Window::Cleanup()
{
	glfwDestroyWindow(GLFWWindow);
}

void Window::CreateWindowSurface(VkInstance Instance, VkSurfaceKHR* Surface)
{
	VK_CHECK(glfwCreateWindowSurface(Instance, GLFWWindow, nullptr, Surface));
}
