// stdafx.cpp : source file that includes just the standard includes
// SpiderChase.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#if defined (PLATFORM_MACOS) || defined (PLATFORM_IOS)

void Log (LogLevel severity, const char* formatString, ...) {
	va_list argumentList;
	va_start(argumentList, formatString);
	
	switch (severity) {
		case LogLevel::Verbose:
			printf ("Verbose: ");
			break;
		case LogLevel::Debug:
			printf ("Debug: ");
			break;
		case LogLevel::Information:
			printf ("Info: ");
			break;
		case LogLevel::Warning:
			printf ("Warning: ");
			break;
		case LogLevel::Error:
			printf ("Error: ");
			break;
		case LogLevel::Critical:
			printf ("Critical: ");
			break;
		case LogLevel::None:
		default:
			break;
	}
	
	vprintf (formatString, argumentList);
	printf ("\n");
	
	va_end(argumentList);
}

#endif //PLATFORM_MACOS || PLATFORM_IOS

namespace Helper {
	
std::string Trim (const std::string& src) {
	if (src.empty ()) {
		return std::string ();
	}

	size_t pos = 0;
	size_t len = src.length ();

	for (size_t i = 0, iEnd = src.length (); i < iEnd; ++i) {
		if (!isspace (src[i])) {
			break;
		}

		++pos;
		--len;
	}

	if (len <= 0) {
		return std::string ();
	}

	for (size_t i = src.length (); i > 0; --i) {
		if (!isspace (src[i - 1])) {
			break;
		}
		--len;
	}

	if (len <= 0) {
		return std::string ();
	}

	return src.substr (pos, len);
}

bool StringStartsWith (const std::string& str, const std::string& find) {
	return str.find (find) == 0;
}

bool StringEndsWith (const std::string& str, const std::string& find) {
	return str.rfind (find) == str.length () - find.length ();
}

} //namespace Helper
