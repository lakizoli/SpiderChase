#pragma once

#include <assimp/IOStream.hpp>
#include "pak.hpp"

class PakIOStream : public Assimp::IOStream {
	Pak::FileEntry _pakEntry;
	size_t _readPos;
	std::vector<uint8_t> _data;

public:
	PakIOStream (const Pak::FileEntry& pakEntry, std::istream& stream);
	virtual ~PakIOStream ();

	virtual size_t Read (void* pvBuffer, size_t pSize, size_t pCount) override;
	virtual size_t Write (const void* pvBuffer, size_t pSize, size_t pCount) override;
	virtual aiReturn Seek (size_t pOffset, aiOrigin pOrigin) override;
	virtual size_t Tell () const override;
	virtual size_t FileSize () const override;
	virtual void Flush () override;
};
