#include <stdexcept>
#include <iostream>

#include "Core/Application.h"
#include "Core/Logger.h"

int main()
{
	Application App = Application("Blur Engine", 1280, 720);

	Logger::Init();
	BE_TRACE("Log Initialized!");

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