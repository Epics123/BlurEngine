#pragma once

#include "spdlog/spdlog.h"

class Logger
{
public:
	static void Init();

	inline static class std::shared_ptr<spdlog::logger>& GetCoreLogger() { return sCoreLogger; }

private:
	static class std::shared_ptr<spdlog::logger> sCoreLogger;
};

// core log macros
#define BE_TRACE(...) ::Logger::GetCoreLogger()->trace(__VA_ARGS__);
#define BE_INFO(...)  ::Logger::GetCoreLogger()->info(__VA_ARGS__);
#define BE_WARN(...)  ::Logger::GetCoreLogger()->warn(__VA_ARGS__);
#define BE_ERROR(...) ::Logger::GetCoreLogger()->error(__VA_ARGS__);
#define BE_CRITICAL(...) ::Logger::GetCoreLogger()->critical(__VA_ARGS__);