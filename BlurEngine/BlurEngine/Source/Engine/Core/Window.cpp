#include "Window.h"

Window::Window(const std::string& WindowName, const uint32_t WindowWidth, const uint32_t WindowHeight, const bool CanResize)
	:Name{WindowName}, Width{WindowWidth}, Height{WindowHeight}, Resizeable{CanResize}
{
	
}

void Window::Init(GLFWkeyfun keyCallback, GLFWcursorposfun cursorPosCallback, GLFWmousebuttonfun mouseButtonCallback, GLFWscrollfun mouseScrollCallback, GLFWframebuffersizefun framebufferResizeCallback, void* user)
{
	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, Resizeable);

	GLFWWindow = glfwCreateWindow(Width, Height, Name.c_str(), nullptr, nullptr);

	glfwSetKeyCallback(GLFWWindow, keyCallback);
	glfwSetCursorPosCallback(GLFWWindow, cursorPosCallback);
	glfwSetMouseButtonCallback(GLFWWindow, mouseButtonCallback);
	glfwSetScrollCallback(GLFWWindow, mouseScrollCallback);

	glfwSetWindowUserPointer(GLFWWindow, this);
	glfwSetFramebufferSizeCallback(GLFWWindow, framebufferResizeCallback);
}

void Window::Cleanup()
{
	glfwDestroyWindow(GLFWWindow);
}
