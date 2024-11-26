#pragma once

#include "../VulkanCore/Utility.h"
#include "../VulkanCore/Context.h"
#include "Window.h"

#include <memory>

class Renderer
{
public:
	MOVABLE_ONLY(Renderer)

	Renderer();
	~Renderer();

	void Init(std::shared_ptr<class Window> AppWindow);

private:
	std::unique_ptr<VulkanCore::Context> RenderingContext;

	std::shared_ptr<class Window> ActiveWindow;
};