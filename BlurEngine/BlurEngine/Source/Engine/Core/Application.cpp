#include "Application.h"

float gDeltaTime = 0.0f;

Application::Application()
{
	Application("BlurEngine", 1280, 720);
}

Application::Application(const std::string& AppName, const uint32_t AppWidth, const uint32_t AppHeight)
	:Name{AppName}, Width{AppWidth}, Height{AppHeight}
{
	AppWindow = std::make_unique<Window>(Name, Width, Height, true);
}

void Application::Run()
{
	if(AppWindow)
	{
		AppWindow->Init(KeyCallback, CursorPosCallback, MouseButtonCallback, ScrollCallback, FramebufferResizeCallback, this);

		Tick();
	}
}

void Application::Tick()
{
	LastFrameTime = (float)glfwGetTime();
	while (!glfwWindowShouldClose(AppWindow->GetGLFWWindow()))
	{
		const float CurrentFrameTime = (float)glfwGetTime();
		gDeltaTime = CurrentFrameTime - LastFrameTime;
		LastFrameTime = CurrentFrameTime;

		glfwPollEvents();
		ProcessInput(AppWindow->GetGLFWWindow());
	}
}

void Application::KeyCallback(GLFWwindow* Window, int Key, int Scancode, int Action, int Mods)
{
	if (Key == GLFW_KEY_ESCAPE && Action == GLFW_PRESS)
		glfwSetWindowShouldClose(Window, true);
}

void Application::CursorPosCallback(GLFWwindow* Window, double XPos, double YPos)
{
	
}

void Application::MouseButtonCallback(GLFWwindow* Window, int Button, int Action, int Mods)
{
	if (Button == GLFW_MOUSE_BUTTON_RIGHT && Action == GLFW_PRESS)
	{
		//moveCamera = true;
		glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	if (Button == GLFW_MOUSE_BUTTON_RIGHT && Action == GLFW_RELEASE)
	{
		//moveCamera = false;
		glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void Application::ScrollCallback(GLFWwindow* Window, double XOffset, double YOffset)
{
	
}

void Application::FramebufferResizeCallback(GLFWwindow* InWindow, int InWidth, int InHeight)
{
	Window* WindowPtr = reinterpret_cast<Window*>(glfwGetWindowUserPointer(InWindow));
	WindowPtr->SetFrameBufferResized(true);
	WindowPtr->SetWidth(InWidth);
	WindowPtr->SetHeight(InHeight);
}

void Application::ProcessInput(GLFWwindow* ActiveWindow)
{
	
}
