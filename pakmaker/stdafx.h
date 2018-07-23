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

#else
#	error "OS not implemented!"
#endif

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

//Helper functions

namespace Helper {

std::string Trim (const std::string& src);
uint64_t GetFileLength (std::istream& stream);
bool CopyFile (std::istream& src, std::ostream& dest, uint64_t byteCount);
bool CopyFile (std::istream& stream, uint64_t byteCount, const fs::path& path);
bool CopyFile (const fs::path& path, std::ostream& stream);

} //namespace Helper
