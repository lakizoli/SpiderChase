#include "stdafx.h"
#include "Platform.hpp"

namespace Platform {

bool RenameFile (const std::string& srcPath, const std::string& destPath) {
#ifdef PLATFORM_WINDOWS
	std::error_code err;
	fs::rename (fs::path (srcPath), fs::path (destPath), err);
	if (err) {
		return false;
	}
	
	return true;
#elif defined (PLATFORM_MACOS)
	boost::system::error_code err;
	try {
		fs::rename (fs::path (srcPath), fs::path (destPath));
	} catch (...) {
		return false;
	}
	
	return true;
#else
#	error "OS not implemented!"
#endif
}

bool RemoveFile (const std::string& path) {
#ifdef PLATFORM_WINDOWS
	std::error_code err;
#elif defined (PLATFORM_MACOS)
	boost::system::error_code err;
#else
#	error "OS not implemented!"
#endif

	fs::remove (fs::path (path), err);
	if (err) {
		return false;
	}
	
	return false;
}

} //namespace Platform
