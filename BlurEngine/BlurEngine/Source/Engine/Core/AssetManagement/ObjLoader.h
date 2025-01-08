#pragma once

#include "VulkanCommon.h"
#include "../Runtime/Model.h"

namespace EngineCore
{
	
class OBJLoader
{
public:
	std::shared_ptr<StaticMesh> Load(const std::vector<char>& Buffer);
	std::shared_ptr<StaticMesh> Load(const std::string& Filepath);
	// TODO: Multithreaded loading;

private:
	void CalculateAABB(Model& OutModel, const Vertex& InVertex);
	void CalculateTangentBasis(Model& OutModel);
};

}