#pragma once
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace TRE
{
	class Log
	{
		public:
			static void Init();
		
			static std::shared_ptr<spdlog::logger>& GetCoreLogger();
			static std::shared_ptr<spdlog::logger>& GetClientLogger();

		private:
			static std::shared_ptr<spdlog::logger> s_CoreLogger;
			static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Core log macros
#define TRE_CORE_TRACE(...)    ::TRE::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define TRE_CORE_INFO(...)     ::TRE::Log::GetCoreLogger()->info(__VA_ARGS__)
#define TRE_CORE_WARN(...)     ::TRE::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define TRE_CORE_ERROR(...)    ::TRE::Log::GetCoreLogger()->error(__VA_ARGS__)
#define TRE_CORE_CRITICAL(...) ::TRE::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define TRE_TRACE(...)         ::TRE::Log::GetClientLogger()->trace(__VA_ARGS__)
#define TRE_INFO(...)          ::TRE::Log::GetClientLogger()->info(__VA_ARGS__)
#define TRE_WARN(...)          ::TRE::Log::GetClientLogger()->warn(__VA_ARGS__)
#define TRE_ERROR(...)         ::TRE::Log::GetClientLogger()->error(__VA_ARGS__)
#define TRE_CRITICAL(...)      ::TRE::Log::GetClientLogger()->critical(__VA_ARGS__)