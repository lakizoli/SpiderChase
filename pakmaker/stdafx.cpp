// stdafx.cpp : source file that includes just the standard includes
// pakmaker.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

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

uint64_t GetFileLength (std::istream& stream) {
	uint64_t cur = stream.tellg ();
	if (!stream) {
		return 0;
	}

	stream.seekg (0, std::ios::end);
	if (!stream) {
		return 0;
	}

	uint64_t len = stream.tellg ();
	if (!stream) {
		return 0;
	}

	stream.seekg (cur);
	return len;
}

bool CopyFile (std::istream& src, std::ostream& dest, uint64_t byteCount) {
	const uint64_t chunkSize = 1024 * 1024;
	uint64_t chunkCount = byteCount / chunkSize;
	if (byteCount % chunkSize > 0) {
		++chunkCount;
	}

	std::vector<uint8_t> buffer (chunkSize);
	while (chunkCount--) {
		uint64_t readSize = chunkCount == 0 ? byteCount % chunkSize : chunkSize;
		if (readSize <= 0) {
			continue;
		}

		src.read ((char*) &buffer[0], readSize);
		if (!src) {
			std::cout << "Error: Read error (copy)!" << std::endl;
			return false;
		}

		dest.write ((const char*) &buffer[0], readSize);
		if (!dest) {
			std::cout << "Error: Write error (copy)!" << std::endl;
			return false;
		}
	}

	return true;
}

bool CopyFile (std::istream& stream, uint64_t byteCount, const fs::path& path) {
	std::fstream dest (path.string (), std::ios::binary | std::ios::out | std::ios::trunc);
	if (!dest) {
		std::cout << "Error: Cannot create file! path -> " << path.string () << std::endl;
		return false;
	}

	return CopyFile (stream, dest, byteCount);
}

bool CopyFile (const fs::path& path, std::ostream& stream) {
	std::fstream src (path.string (), std::ios::binary | std::ios::in);
	if (!src) {
		std::cout << "Error: Cannot open file!" << std::endl;
		return false;
	}

	uint64_t srcLen = GetFileLength (src);
	if (!src) {
		std::cout << "Error: Read error (1)!" << std::endl;
		return false;
	}

	return CopyFile (src, stream, srcLen);
}

} //namespace Helper
