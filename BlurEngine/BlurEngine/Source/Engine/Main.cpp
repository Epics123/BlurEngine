#include <stdexcept>
#include <iostream>

#include "Core/Application.h"

int main()
{
	Application App = Application("Blur Engine", 1280, 720);

	try
	{
		App.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;

		system("pause");

		return EXIT_FAILURE;
	}

	system("pause");
	return EXIT_SUCCESS;
}