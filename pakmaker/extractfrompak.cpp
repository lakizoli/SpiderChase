#include "stdafx.h"
#include "pak.hpp"

int ExtractFromPak (const std::string& pakPath, const std::string& destPath, const std::vector<std::string>& extractFiles) {
	//Gather files to extract
	std::set<std::string> filesToExtract;
	for (std::string extractFile : extractFiles) {
		std::transform (extractFile.begin (), extractFile.end (), extractFile.begin (), ::tolower);
		filesToExtract.insert (extractFile);
	}

	//Extract files
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
	if (files.size () > 0) {
		fs::path dest (destPath);
		if (!fs::exists (dest)) {
			if (!fs::create_directories (dest)) {
				std::cout << "Cannot create destination directory -> " << dest.string () << std::endl << std::endl;
				return EXIT_FAILURE;
			}
		}

		for (const Pak::FileEntry& file : files) {
			if (filesToExtract.find (std::string (file.path)) == filesToExtract.end ()) {
				continue;
			}

			std::cout << "Unpacking " << std::string (file.path) << std::endl;

			fs::path filePath = destPath / fs::path (std::string (file.path));
			fs::path fileDir = filePath.parent_path ();

			if (!fs::exists (fileDir)) {
				if (!fs::create_directories (fileDir)) {
					std::cout << "Error: cannot create directory -> " << fileDir.string () << std::endl << std::endl;
					return EXIT_FAILURE;
				}
			}

			if (!pak->ReadFile (file, [&file, &filePath] (std::istream& stream) -> bool {
				return Helper::CopyFile (stream, file.size, filePath);
			})) {
				std::cout << "Error: cannot extract file -> " << std::string (file.path) << std::endl << std::endl;
				return EXIT_FAILURE;
			}

			size += file.size;
			++cnt;
		}
	}

	std::cout << "Extracted: " << size << " bytes in " << cnt << " files" << std::endl << std::endl;
	return EXIT_SUCCESS;
}
