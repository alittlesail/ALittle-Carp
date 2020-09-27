#ifndef CARP_SYSTEM_INCLUDED
#define CARP_SYSTEM_INCLUDED (1)

#include <string>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

class CarpSystem
{
public:
	static std::string GetPlatform()
	{
#ifdef _WIN32
		return "Windows";
#elif __ANDROID__
		return "Andoird";
#elif __APPLE__
#ifdef TARGET_OS_IPHONE
		return "iOS";
#else
		return "Mac";
#endif
#elif __EMSCRIPTEN__
		return "Emscripten";
#endif
		return "";
	}
};

#endif