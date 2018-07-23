#include "stdafx.h"
#include "pak.hpp"

struct PackFile {
	std::string name;
	fs::path path;

	PackFile () {}

	PackFile (const std::string& line) {
		if (line.empty ()) {
			return;
		}

		size_t pos = line.find ('=');
		if (pos == std::string::npos) {
			return;
		}

		if (line.length () <= pos + 1) {
			return;
		}

		path = Helper::Trim (line.substr (0, pos));
		name = Helper::Trim (line.substr (pos + 1));
	}

	void ConvertRelativePathToAbsolute (const fs::path& basePath) {
		if (!path.has_root_path ()) { //File specified relative to the list file's path
			path = basePath / path;
		}
	}
};

int Pack (const std::string& pakPath, const std::string& listFilePath) {
	//Gather files to pack
	fs::path pathList (listFilePath);
	if (!fs::exists (pathList)) {
		std::cout << "Error: the list file not found at path -> " << pathList.string () << std::endl << std::endl;
		return EXIT_FAILURE;
	}

	std::fstream listFile (pathList.string (), std::ios::binary | std::ios::in);
	std::vector<std::string> filesToPack;
	std::map<std::string, PackFile> files;
	for (std::string line; std::getline (listFile, line); ) {
		std::transform (line.begin (), line.end (), line.begin (), ::tolower);

		PackFile pack (line);
		if (pack.name.empty ()) {
			std::cout << "Warning: Cannot parse line from file [ignored] -> " << line << std::endl;
			continue;
		}

		if (files.find (pack.name) != files.end ()) {
			std::cout << "Warning: Duplicate file found in line [ignored] -> " << line << std::endl;
			continue;
		}

		pack.ConvertRelativePathToAbsolute (pathList.parent_path ());

		if (!fs::exists (pack.path)) {
			std::cout << "Warning: file not found! [ignored] -> " << line << std::endl;
			continue;
		}

		if (fs::is_directory (pack.path)) { //Add directory to the pak (recursive)
			std::string dirPath = pack.path.string ();
			for (auto& child : fs::recursive_directory_iterator (pack.path)) {
				if (fs::is_directory (child)) {
					continue;
				}

				PackFile childPack;
				childPack.path = child.path ();
				childPack.name = pack.name + '/' + childPack.path.string ().substr (dirPath.length () + 1); //Ingore path separator at start also

				std::transform (childPack.name.begin (), childPack.name.end (), childPack.name.begin (), ::tolower);
				std::replace (childPack.name.begin (), childPack.name.end (), '\\', '/');

				filesToPack.push_back (childPack.name);
				files.emplace (childPack.name, childPack);
			}
		} else { //Add file to the pak
			filesToPack.push_back (pack.name);
			files.emplace (pack.name, pack);
		}
	}

	//Create empty pak file
	fs::path pathPak (pakPath);
	if (!Pak::CreateEmpty (pathPak.string ())) {
		std::cout << "Error: cannot create pak at path -> " << pathPak << std::endl << std::endl;
		return EXIT_FAILURE;
	}

	//Add files to the pak
	if (!Pak::AddFiles (pathPak.string (), filesToPack, [&files] (const std::string& fileName, std::ostream& stream) -> bool {
		std::cout << "Adding " << fileName << std::endl;

		auto it = files.find (fileName);
		if (it == files.end ()) {
			std::cout << "Error: file info not found!" << std::endl;
			return false;
		}

		//Add file to the pak
		const PackFile& file = it->second;
		return Helper::CopyFile (file.path, stream);
	})) {
		std::cout << "Error during add files to pak at path -> " << pathPak << std::endl << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Pack succeeded!" << std::endl << std::endl;
	return EXIT_SUCCESS;
}
