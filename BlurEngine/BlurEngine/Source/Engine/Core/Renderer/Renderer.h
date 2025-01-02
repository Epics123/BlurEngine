#pragma once

#include "../VulkanCore/Utility.h"
#include "../VulkanCore/Context.h"
#include "../VulkanCore/Pipeline.h"
#include "../VulkanCore/CommandQueueManager.h"
#include "Window.h"

#include <memory>
#include <filesystem>

class Renderer
{
public:
	MOVABLE_ONLY(Renderer)

	Renderer();
	~Renderer();

	void Init(std::shared_ptr<class Window> AppWindow);
	void Draw(float DeltaTime);

	void DeviceWaitIdle();
	void WaitForAllSubmits();

	void HandleWindowResized();

public:
	static std::filesystem::path sShaderDirectory;

private:
	uint32_t FramesInFlight;

	std::unique_ptr<VulkanCore::Context> RenderingContext;
	std::shared_ptr<class Window> ActiveWindow;

	std::shared_ptr<VulkanCore::Pipeline> GraphicsPipeline;

	std::unique_ptr<VulkanCore::CommandQueueManager> GraphicsCommandManager;

	std::shared_ptr<VulkanCore::RenderPass> SimpleTrianglePass;
	VkRect2D RenderArea;
};