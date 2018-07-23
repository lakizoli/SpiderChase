#include "stdafx.h"
#include "pakiostream.hpp"

PakIOStream::PakIOStream (const Pak::FileEntry& pakEntry, std::istream& stream) :
	_pakEntry (pakEntry),
	_readPos (0)
{
	if (_pakEntry.size > 0) {
		_data.resize (_pakEntry.size);
		stream.read ((char*)&_data[0], _pakEntry.size);
	}
}

PakIOStream::~PakIOStream () {
}

size_t PakIOStream::Read (void* pvBuffer, size_t pSize, size_t pCount) {
	size_t count = pSize * pCount;
	size_t remaining = _pakEntry.size - _readPos;
	if (count > remaining) {
		count = remaining;
	}

	if (count > 0) {
		std::memcpy (pvBuffer, &_data[_readPos], count);
		_readPos += count;
	}

	return count;
}

size_t PakIOStream::Write (const void* pvBuffer, size_t pSize, size_t pCount) {
	// Not implemented...
	return 0;
}

aiReturn PakIOStream::Seek (size_t pOffset, aiOrigin pOrigin) {
	switch (pOrigin) {
	case aiOrigin_SET:
		_readPos = pOffset;
		if (_readPos > _pakEntry.size) {
			_readPos = _pakEntry.size;
		}
		break;
	case aiOrigin_CUR:
		_readPos += pOffset;
		if (_readPos > _pakEntry.size) {
			_readPos = _pakEntry.size;
		}
		break;
	case aiOrigin_END:
		if (pOffset > _pakEntry.size) {
			_readPos = 0;
		} else {
			_readPos = _pakEntry.size - pOffset;
		}
		break;
	default:
		return aiReturn_FAILURE;
	}

	return aiReturn_SUCCESS;
}

size_t PakIOStream::Tell () const {
	return _readPos;
}

size_t PakIOStream::FileSize () const {
	return _pakEntry.size;
}

void PakIOStream::Flush () {
	// Not implemented...
}
