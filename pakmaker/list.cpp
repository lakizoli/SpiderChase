#include "stdafx.h"
#include "pak.hpp"

int List (const std::string& pakPath) {
	std::shared_ptr<Pak> pak = Pak::OpenForRead (pakPath);
	if (pak == nullptr) {
		std::cout << "Cannot open pak at path -> " << pakPath << std::endl << std::endl;
		return EXIT_FAILURE;
	}

	std::vector<Pak::FileEntry> files;
	if (!pak->ReadDirectory (files)) {
		std::cout << "Cannot read directory from pak at path -> " << pakPath << std::endl << std::endl;
		return EXIT_FAILURE;
	}

	uint64_t cnt = 0;
	uint64_t size = 0;
	for (const Pak::FileEntry& file : files) {
		std::cout << std::string (file.path) << std::endl;
		size += file.size;
		++cnt;
	}

	std::cout << size << " bytes in " << cnt << " files" << std::endl << std::endl;
	return EXIT_SUCCESS;
}
