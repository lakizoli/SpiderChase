#pragma once

#include <assimp/IOSystem.hpp>
#include "pak.hpp"

class PakIOSystem : public Assimp::IOSystem {
	std::shared_ptr<Pak> _pak;
	std::vector<Pak::FileEntry> _entries;
	std::string _masterFile;

	std::string GetFilePath (const std::string& fileName) const;

public:
	PakIOSystem (std::shared_ptr<Pak> pak, const std::vector<Pak::FileEntry>& entries, const std::string& masterFile);
	virtual ~PakIOSystem ();

	virtual bool Exists (const char* pFile) const override;

	virtual char getOsSeparator () const override {
		return '/';
	}

	virtual Assimp::IOStream* Open (const char* pFile, const char* pMode = "rb") override;
	virtual void Close (Assimp::IOStream* pFile) override;
};
