#include "stdafx.h"
#include "pakiosystem.hpp"
#include "pakiostream.hpp"

std::string PakIOSystem::GetFilePath (const std::string& fileName) const {
	if (_masterFile == fileName) {
		return _masterFile;
	}
	
	return _masterFile + '/' + fileName;
}

PakIOSystem::PakIOSystem (std::shared_ptr<Pak> pak, const std::vector<Pak::FileEntry>& entries, const std::string& masterFile) :
	_pak (pak),
	_entries (entries),
	_masterFile (masterFile)
{
}

PakIOSystem::~PakIOSystem () {
}

bool PakIOSystem::Exists (const char* pFile) const {
	std::string searchPath = GetFilePath (pFile);
	for (const Pak::FileEntry& entry : _entries) {
		if (entry.path == searchPath) {
			return true;
		}
	}

	return false;
}

Assimp::IOStream* PakIOSystem::Open (const char* pFile, const char* pMode) {
	std::string searchPath = GetFilePath (pFile);
	for (const Pak::FileEntry& entry : _entries) {
		if (entry.path == searchPath) {
			Assimp::IOStream* result = nullptr;

			if (!_pak->ReadFile (entry, [&result, entry] (std::istream& stream) -> void {
				result = new PakIOStream (entry, stream);
			})) {
				return nullptr;
			}

			return result;
		}
	}

	return nullptr;
}

void PakIOSystem::Close (Assimp::IOStream* pFile) {
	delete (PakIOStream*) pFile;
}
