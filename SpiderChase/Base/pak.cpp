#include "stdafx.h"
#include "Pak.hpp"

using namespace std;

static istream& operator >> (istream& stream, Pak::Header& header) {
	stream.read (header.mimeID, 4);
	stream.read ((char*) &header.offset, sizeof (header.offset));
	stream.read ((char*) &header.size, sizeof (header.size));

	return stream;
}

static istream& operator >> (istream& stream, Pak::FileEntry& entry) {
	stream.read (entry.path, 56);
	stream.read ((char*) &entry.offset, sizeof (entry.offset));
	stream.read ((char*) &entry.size, sizeof (entry.size));

	return stream;
}

Pak::Header::Header () : mimeID { 0,0,0,0 }, offset (0), size (0) {
}

Pak::FileEntry::FileEntry () : offset (0), size (0) {
	memset (path, 0, sizeof (path));
}

Pak::Pak (const string& path) : _path (path), _stream (path, ios::binary | ios::in) {
	if (_stream) {
		_stream.seekg (0);
		_stream >> _header;
	}
}

Pak::~Pak () {
}

bool Pak::IsValid () const {
	return _header.mimeID[0] == 'P' && _header.mimeID[1] == 'A' && _header.mimeID[2] == 'C' && _header.mimeID[3] == 'K';
}

void Pak::EnumerateDirectory (function<bool (const FileEntry& entry)> callback) {
	if (_stream) {
		_stream.seekg (_header.offset);

		while (_stream && _stream.tellg () < _header.offset + _header.size) {
			FileEntry entry;
			_stream >> entry;
			if (!callback (entry)) {
				break;
			}
		}
	}
}

bool Pak::ReadFile (const string& path, function<bool (const string& path, istream& stream)> callback) {
	if (_stream) {
		_stream.seekg (_header.offset);

		while (_stream && _stream.tellg () < _header.offset + _header.size) {
			FileEntry entry;
			_stream >> entry;
			if (string (entry.path) == path) {
				_stream.seekg (entry.offset);

				//Call the read callback
				if (!callback (path, _stream)) {
					return false;
				}

				//Check size after read
				return _stream.tellg () <= entry.offset + entry.size;
			}
		}
	}

	return false;
}
