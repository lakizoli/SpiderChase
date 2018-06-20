// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

//Have to be the first include!
#include "platformconfig.hpp"

#include <string>
#include <vector>
#include <map>
#include <set>
#include <deque>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <cctype>
#include <memory>

#ifdef PLATFORM_WINDOWS

#	include <filesystem>
	namespace fs = std::experimental::filesystem;

#elif defined (PLATFORM_MACOS) || defined (PLATFORM_IOS)

#	include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

enum class LogLevel
{
	Verbose = 0,
	Debug = 1,
	Information = 2,
	Warning = 3,
	Error = 4,
	Critical = 5,
	None = 100,
};

void Log (LogLevel severity, const char* formatString, ...);

#else
#	error "OS not implemented!"
#endif

//Helper functions

std::string Trim (const std::string& src);
bool StringStartsWith (const std::string& str, const std::string& find);
bool StringEndsWith (const std::string& str, const std::string& find);
