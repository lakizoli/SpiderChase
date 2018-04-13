// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _CRT_SECURE_NO_WARNINGS

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

//Helper functions

namespace fs = std::experimental::filesystem;

std::string Trim (const std::string& src);
bool StringStartsWith (const std::string& str, const std::string& find);
bool StringEndsWith (const std::string& str, const std::string& find);
