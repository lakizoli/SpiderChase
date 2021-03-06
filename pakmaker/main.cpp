// pakmaker.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

static int PrintUsage (const char* exePath) {
	fs::path path (exePath);

	std::cout << "Usage:" << std::endl << std::endl <<
		"    " << path.filename () << " <command> <pak path> [parameters]" << std::endl << std::endl <<
		"commands:" << std::endl << std::endl <<
		"    pack    - the list of files specified in the text file given in the first parameter." << std::endl <<
		"    unpack  - unpack the specified .pak file to the destination path given in the first parameter." << std::endl <<
		"    add     - add all of the files and dirs specified in the parameters. May give multiple files at once." << std::endl <<
		"    delete  - delete the files specified in the parameters. May give multiple file at once." << std::endl <<
		"    extract - extract the files from the .pak to the destination path given in parameter." << std::endl <<
		"              Multiple files may be specified after the destination path parameter." << std::endl <<
		"    list    - list the content of the .pak file. No additional parameters needed." << std::endl << std::endl;

	return EXIT_FAILURE;
}

static std::vector<std::string> ReadFileParameters (int startIndex, int argc, const char* argv[]) {
	std::vector<std::string> addFiles;
	for (int i = startIndex; i < argc; ++i) {
		addFiles.push_back (argv[i]);
	}
	return addFiles;
}

extern int Pack (const std::string& pakPath, const std::string& listFilePath);
extern int Unpack (const std::string& pakPath, const std::string& destPath);
extern int List (const std::string& pakPath);
extern int AddToPak (const std::string& pakPath, const std::vector<std::string>& addFiles);
extern int DeleteFromPak (const std::string& pakPath, const std::vector<std::string>& delFiles);
extern int ExtractFromPak (const std::string& pakPath, const std::string& destPath, const std::vector<std::string>& extractFiles);

int main (int argc, const char* argv[]) {
	if (argc < 3) {
		return PrintUsage (argc < 1 ? "pakmaker" : argv[0]);
	}

	std::string cmd = argv[1];
	if (cmd == "pack") { //Pack all files from dir, or from the given list file
		if (argc < 4) {
			std::cout << "Parameter error! The list file not specified!" << std::endl << std::endl;
			return PrintUsage (argv[0]);
		}

		std::string pakPath = argv[2];
		std::string listFilePath = argv[3];
		return Pack (pakPath, listFilePath);
	} else if (cmd == "unpack") { //Unpack the whole pak file to dir
		if (argc < 4) {
			std::cout << "Parameter error! The destination path not specified!" << std::endl << std::endl;
			return PrintUsage (argv[0]);
		}

		std::string pakPath = argv[2];
		std::string destPath = argv[3];
		return Unpack (pakPath, destPath);
	} else if (cmd == "add") { //Add files or dirs to the existing pack
		if (argc < 4) {
			std::cout << "Parameter error! No files specified!" << std::endl << std::endl;
			return PrintUsage (argv[0]);
		}

		std::string pakPath = argv[2];
		std::vector<std::string> addFiles = ReadFileParameters (3, argc, argv);
		return AddToPak (pakPath, addFiles);
	} else if (cmd == "delete") { //Delete items from the existing pack
		if (argc < 4) {
			std::cout << "Parameter error! No files specified!" << std::endl << std::endl;
			return PrintUsage (argv[0]);
		}

		std::string pakPath = argv[2];
		std::vector<std::string> delFiles = ReadFileParameters (3, argc, argv);
		return DeleteFromPak (pakPath, delFiles);
	} else if (cmd == "extract") { //Extract items from the existing pack
		if (argc < 4) {
			std::cout << "Parameter error! The destination path not specified!" << std::endl << std::endl;
			return PrintUsage (argv[0]);
		}

		if (argc < 5) {
			std::cout << "Parameter error! No files specified!" << std::endl << std::endl;
			return PrintUsage (argv[0]);
		}

		std::string pakPath = argv[2];
		std::string destPath = argv[3];
		std::vector<std::string> extractFiles = ReadFileParameters (4, argc, argv);
		return ExtractFromPak (pakPath, destPath, extractFiles);
	} else if (cmd == "list") { //List the pack's content
		std::string pakPath = argv[2];
		return List (pakPath);
	}

	return EXIT_FAILURE;
}

