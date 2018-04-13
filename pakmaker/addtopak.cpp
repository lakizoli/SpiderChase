#include "stdafx.h"
#include "pak.hpp"

int AddToPak (const std::string& pakPath, const std::vector<std::string>& addFiles) {
	//Gather files to add
	std::vector<std::string> filesToPack;
	std::map<std::string, fs::path> files;
	for (const std::string& addFile : addFiles) {
		fs::path addFilePath (addFile);

		std::string packName = addFilePath.filename ().string ();
		if (packName.empty ()) {
			std::cout << "Warning: file name empty [ignored] -> " << addFilePath.string () << std::endl;
			continue;
		}

		std::transform (packName.begin (), packName.end (), packName.begin (), ::tolower);
		if (files.find (packName) != files.end ()) {
			std::cout << "Warning: duplicate file found [ignored] -> " << addFilePath.string () << std::endl;
			continue;
		}

		if (!fs::exists (addFilePath)) {
			std::cout << "Warning: file not found! [ignored] -> " << addFilePath.string () << std::endl;
			continue;
		}

		if (fs::is_directory (addFilePath)) { //Add directory to the pak (recursive)
			std::string dirPath = addFilePath.string ();
			for (auto& child : fs::recursive_directory_iterator (addFilePath)) {
				if (fs::is_directory (child)) {
					continue;
				}

				fs::path childPackPath = child.path ();
				std::string childPackName = packName + '/' + childPackPath.string ().substr (dirPath.length () + 1); //Ingore path separator at start also

				std::transform (childPackName.begin (), childPackName.end (), childPackName.begin (), ::tolower);
				std::replace (childPackName.begin (), childPackName.end (), '\\', '/');

				filesToPack.push_back (childPackName);
				files.emplace (childPackName, childPackPath);
			}
		} else { //Add file to the pak
			filesToPack.push_back (packName);
			files.emplace (packName, addFilePath);
		}
	}

	//Add files to the pak
	fs::path pathPak (pakPath);
	if (!Pak::AddFiles (pathPak.string (), filesToPack, [&files] (const std::string& fileName, std::ostream& stream) -> bool {
		std::cout << "Adding " << fileName << std::endl;

		auto it = files.find (fileName);
		if (it == files.end ()) {
			std::cout << "Error: file info not found!" << std::endl;
			return false;
		}

		//Add file to the pak
		const fs::path& filePath = it->second;
		return CopyFile (filePath, stream);
	})) {
		std::cout << "Error during add files to pak at path -> " << pathPak << std::endl << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Add succeeded!" << std::endl << std::endl;
	return EXIT_SUCCESS;
}
