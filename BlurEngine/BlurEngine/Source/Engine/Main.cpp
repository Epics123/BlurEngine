#include <stdexcept>
#include <iostream>
#include <filesystem>

#include "Core/Application.h"
#include "Core/Logger.h"

int main(int argc, char** argv)
{
	Application App = Application("Blur Engine", 1280, 720);
	std::filesystem::current_path(App.WorkingDirectory);

	Logger::Init();
	BE_TRACE("Log Initialized!");
	BE_WARN(std::filesystem::current_path().string());

	try
	{
		App.Run();
	}
	catch (const std::exception& e)
	{
		BE_ERROR(e.what());

		system("pause");

		return EXIT_FAILURE;
	}

	system("pause");
	return EXIT_SUCCESS;
}