#pragma once

#include "../VulkanCore/Utility.h"

#include <memory>

class Renderer
{
public:
	MOVABLE_ONLY(Renderer)

	Renderer();
	~Renderer();

	void Init();

private:
	std::unique_ptr<class Context> RenderingContext;
};