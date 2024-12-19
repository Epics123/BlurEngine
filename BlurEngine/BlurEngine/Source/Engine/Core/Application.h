#pragma once

#include <string>
#include <memory>

#include "Window.h"
#include "Renderer/Renderer.h"

class Application
{
public:
	Application();
	Application(const std::string& AppName, const uint32_t AppWidth, const uint32_t AppHeight);

	void Run();
	void Tick();

private:
	static void KeyCallback(GLFWwindow* Window, int Key, int Scancode, int Action, int Mods);
	static void CursorPosCallback(GLFWwindow* Window, double XPos, double YPos);
	static void MouseButtonCallback(GLFWwindow* Window, int Button, int Action, int Mods);
	static void ScrollCallback(GLFWwindow* Window, double XOffset, double YOffset);
	static void FramebufferResizeCallback(GLFWwindow* InWindow, int InWidth, int InHeight);

	void ProcessInput(GLFWwindow* ActiveWindow);

private:
	std::string Name;

	uint32_t Width;
	uint32_t Height;

	float LastFrameTime;

	std::shared_ptr<class Window> AppWindow = nullptr;
	std::unique_ptr<Renderer> AppRenderer;
};