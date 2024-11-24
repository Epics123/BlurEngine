#pragma once

#include <string>

class Application
{
public:
	Application();
	Application(const std::string& AppName, const uint32_t AppWidth, const uint32_t AppHeight);

	void Run();

private:
	std::string Name;

	uint32_t Width;
	uint32_t Height;
};