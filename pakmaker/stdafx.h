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
#include <filesystem>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <cctype>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

namespace fs = std::experimental::filesystem;

//Helper functions

std::string Trim (const std::string& src);
uint64_t GetFileLength (std::istream& stream);
bool CopyFile (std::istream& src, std::ostream& dest, uint64_t byteCount);
bool CopyFile (std::istream& stream, uint64_t byteCount, const fs::path& path);
bool CopyFile (const fs::path& path, std::ostream& stream);
