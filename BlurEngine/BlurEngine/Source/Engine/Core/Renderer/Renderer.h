#pragma once

#include "../VulkanCore/Utility.h"
#include "../VulkanCore/Context.h"
#include "../VulkanCore/Pipeline.h"
#include "../VulkanCore/CommandQueueManager.h"

#include "../Runtime/Model.h"
#include "../Runtime/Camera.h"

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

	std::shared_ptr<VulkanCore::RenderPass> IndirectDrawPass;
	VkRect2D RenderArea;

	std::vector<std::shared_ptr<EngineCore::StaticMesh>> SceneMeshes; // TODO: Temporary for now until we have some concept of a scene/level

	EngineCore::Camera MainCamera;

	const uint32_t CAMERA_SET = 0;
	const uint32_t TEXTURES_SET = 1;
	const uint32_t SAMPLER_SET = 2;
	const uint32_t STORAGE_BUFFER_SET = 3;  // storing vertex/index/indirect/material buffer in array
	const uint32_t BINDING_0 = 0;
	const uint32_t BINDING_1 = 1;
	const uint32_t BINDING_2 = 2;
	const uint32_t BINDING_3 = 3;
};