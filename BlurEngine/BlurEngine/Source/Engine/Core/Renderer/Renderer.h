#pragma once

#include "../VulkanCore/Utility.h"
#include "../VulkanCore/Context.h"
#include "../VulkanCore/Pipeline.h"
#include "../VulkanCore/CommandQueueManager.h"

#include "../Runtime/Model.h"

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
	static std::filesystem::path sModelDirectory; // TODO: Should probably move these to some sort of asset manager

private:
	uint32_t FramesInFlight;

	std::unique_ptr<VulkanCore::Context> RenderingContext;
	std::shared_ptr<class Window> ActiveWindow;

	std::shared_ptr<VulkanCore::Pipeline> GraphicsPipeline;

	std::unique_ptr<VulkanCore::CommandQueueManager> GraphicsCommandManager;

	std::shared_ptr<VulkanCore::RenderPass> SimpleTrianglePass;
	VkRect2D RenderArea;

	std::vector<std::shared_ptr<EngineCore::StaticMesh>> SceneMeshes; // TODO: Temporary for now until we have some concept of a scene/level
};