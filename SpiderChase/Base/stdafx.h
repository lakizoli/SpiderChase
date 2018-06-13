// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _WINDOWS
#	define _CRT_SECURE_NO_WARNINGS
#endif //_WINDOWS

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

#ifdef _WINDOWS

#	include <filesystem>
	namespace fs = std::experimental::filesystem;

#elif __APPLE__

#	include <boost/filesystem.hpp>
	namespace fs = boost::filesystem;

#else
#	error "OS not implemented!"
#endif

//Helper functions

std::string Trim (const std::string& src);
bool StringStartsWith (const std::string& str, const std::string& find);
bool StringEndsWith (const std::string& str, const std::string& find);
