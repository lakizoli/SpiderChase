#pragma once

#if defined(_WIN32) || defined (_WIN64)
#	define PLATFORM_WINDOWS
#elif defined (__APPLE__)
#	include "TargetConditionals.h"
#	if defined (TARGET_OS_MAC) // Other kinds of Mac OS
#		define PLATFORM_MACOS
#	elif defined (TARGET_OS_IPHONE) // iOS device
#		define PLATFORM_IOS
#	elif defined (TARGET_IPHONE_SIMULATOR) // iOS Simulator
#		error "We don't support the iphone simulator platform at all!"
#	else
#   	error "Unknown Apple platform!"
#	endif
#elif defined (__ANDROID__)
#	define PLATFORM_ANDROID
#else //Unknown platform
#	error "Platform not supported!"
#endif //Platform detection

#ifdef PLATFORM_WINDOWS
#	define _CRT_SECURE_NO_WARNINGS
#endif
