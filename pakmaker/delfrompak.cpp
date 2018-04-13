#include "stdafx.h"
#include "pak.hpp"

int DeleteFromPak (const std::string& pakPath, const std::vector<std::string>& delFiles) {
	//Gather files to delete
	std::set<std::string> filesToDelete;
	for (std::string delFile : delFiles) {
		std::transform (delFile.begin (), delFile.end (), delFile.begin (), ::tolower);
		filesToDelete.insert (delFile);
	}

	//Mark files in the pak to delete
	fs::path pathPak (pakPath);
	if (!Pak::MarkFilesToDelete (pathPak.string (), filesToDelete)) {
		std::cout << "Error during marking files to delete in pak at path -> " << pathPak << std::endl << std::endl;
		return EXIT_FAILURE;
	}

	//Delete marked files
	if (!Pak::DeleteMarkedFiles (pathPak.string ())) {
		std::cout << "Error during deletion of files in pak at path -> " << pathPak << std::endl << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Delete succeeded!" << std::endl << std::endl;
	return EXIT_SUCCESS;
}