#include "Application.h"

Application::Application()
{
	Application("BlurEngine", 1280, 720);
}

Application::Application(const std::string& AppName, const uint32_t AppWidth, const uint32_t AppHeight)
	:Name{AppName}, Width{AppWidth}, Height{AppHeight}
{
	
}

void Application::Run()
{

}
