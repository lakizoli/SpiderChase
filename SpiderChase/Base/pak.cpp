#include "stdafx.h"
#include "pak.hpp"

static std::istream& operator >> (std::istream& stream, Pak::Header& header) {
	stream.read (header.mimeID, 4);
	stream.read ((char*) &header.offset, sizeof (header.offset));
	stream.read ((char*) &header.size, sizeof (header.size));

	return stream;
}

static std::ostream& operator << (std::ostream& stream, const Pak::Header& header) {
	stream.write (header.mimeID, 4);
	stream.write ((const char*) &header.offset, sizeof (header.offset));
	stream.write ((const char*) &header.size, sizeof (header.size));

	return stream;
}

static std::istream& operator >> (std::istream& stream, Pak::FileEntry& entry) {
	stream.read (entry.path, 56);
	stream.read ((char*) &entry.offset, sizeof (entry.offset));
	stream.read ((char*) &entry.size, sizeof (entry.size));

	return stream;
}

static std::ostream& operator << (std::ostream& stream, const Pak::FileEntry& entry) {
	stream.write (entry.path, 56);
	stream.write ((const char*) &entry.offset, sizeof (entry.offset));
	stream.write ((const char*) &entry.size, sizeof (entry.size));

	return stream;
}

static bool OpenPakAndReadHeader (const std::string& path, std::fstream& stream, Pak::Header& header) {
	//Open stream
	stream.open (path, std::ios::binary | std::ios::in | std::ios::out);
	if (!stream) {
		return false;
	}

	//Read header
	stream.seekg (0);
	if (!stream) {
		return false;
	}

	stream >> header;
	return (bool) stream;
}

std::shared_ptr<Pak> Pak::OpenForRead (const std::string& path) {
	std::shared_ptr<Pak> pak (new Pak ());

	pak->_path = path;
	pak->_stream = std::fstream (path, std::ios::binary | std::ios::in);
	if (pak->_stream) {
		pak->_stream.seekg (0);

		if (pak->_stream) {
			pak->_stream >> pak->_header;

			if (pak->_stream && pak->IsValid ()) {
				return pak;
			}
		}
	}

	return nullptr;
}

bool Pak::IsValid () const {
	return _header.mimeID[0] == 'P' && _header.mimeID[1] == 'A' && _header.mimeID[2] == 'C' && _header.mimeID[3] == 'K';
}

bool Pak::ReadDirectory (std::vector<FileEntry>& dir) {
	dir.clear ();

	if (_stream) {
		_stream.seekg (_header.offset);

		while (_stream && _stream.tellg () < _header.offset + _header.size) {
			FileEntry entry;
			_stream >> entry;
			dir.push_back (entry);
		}
	}

	return (bool) _stream;
}

bool Pak::ReadFile (const FileEntry& fileEntry, std::function<void(std::istream&)> callback) {
	if (_stream) {
		_stream.seekg (fileEntry.offset);

		if (_stream) {
			callback (_stream);
		}
	}

	return (bool) _stream;
}

bool Pak::CreateEmpty (const std::string& path) {
	std::fstream stream (path, std::ios::binary | std::ios::out | std::ios::trunc);

	if (stream) {
		Header header;

		header.mimeID[0] = 'P';
		header.mimeID[1] = 'A';
		header.mimeID[2] = 'C';
		header.mimeID[3] = 'K';

		header.offset = sizeof (header);
		header.size = 0;

		stream.write ((const char*) &header, sizeof (header));
	}

	return (bool) stream;
}

bool Pak::AddFiles (const std::string& path, const std::vector<std::string>& names, std::function<void (const std::string&, std::ostream&)> writer) {
	if (names.size () <= 0) {
		return true;
	}

	//Open pak and read header
	std::fstream stream;
	Header header;
	if (!OpenPakAndReadHeader (path, stream, header)) {
		return false;
	}

	//Read directory
	stream.seekg (header.offset);
	if (!stream) {
		return false;
	}

	std::vector<FileEntry> dir;
	for (uint32_t i = 0, iEnd = header.size / sizeof (FileEntry); i < iEnd; ++i) {
		FileEntry entry;

		stream >> entry;
		if (!stream) {
			return false;
		}

		dir.push_back (entry);
	}

	//Move to the start offset of the directory
	stream.seekp (header.offset);
	if (!stream) {
		return false;
	}

	//Write out all files
	for (const std::string& name : names) {
		uint32_t offset = (uint32_t) stream.tellp ();
		if (!stream) {
			return false;
		}

		writer (name, stream);
		if (!stream) {
			return false;
		}

		uint32_t size = (uint32_t)stream.tellp () - offset;
		if (!stream) {
			return false;
		}

		if (size <= 0) { //Leave out empty files
			continue;
		}

		FileEntry entry;
		strncpy (entry.path, name.c_str (), 55);
		entry.offset = offset;
		entry.size = size;

		dir.push_back (entry);
	}

	//Write new directory
	header.offset = (uint32_t) stream.tellp ();
	if (!stream) {
		return false;
	}

	header.size = (uint32_t) dir.size () * sizeof (FileEntry);

	for (size_t i = 0, iEnd = dir.size (); i < iEnd; ++i) {
		stream << dir[i];
		if (!stream) {
			return false;
		}
	}

	//Write header
	stream.seekp (0);
	if (!stream) {
		return false;
	}

	stream << header;

	return (bool) stream;
}

bool Pak::MarkFilesToDelete (const std::string& path, const std::set<std::string>& names) {
	if (names.size () <= 0) {
		return true;
	}

	//Open pak and read header
	std::fstream stream;
	Header header;
	if (!OpenPakAndReadHeader (path, stream, header)) {
		return false;
	}

	//Enumerate directory and mark matching files
	uint64_t entryOffset = header.offset;
	for (uint32_t i = 0, iEnd = header.size / sizeof (FileEntry); i < iEnd; ++i) {
		stream.seekg (entryOffset);
		if (!stream) {
			return false;
		}

		FileEntry entry;
		stream >> entry;
		if (!stream) {
			return false;
		}

		if (names.find (std::string (entry.path)) != names.end ()) {
			stream.seekp (entryOffset);
			if (!stream) {
				return false;
			}

			stream.write ("***\0", 4); //Write mark sign to start of the name
			if (!stream) {
				return false;
			}
		}

		entryOffset += sizeof (FileEntry);
	}

	return true;
}

bool Pak::DeleteMarkedFiles (const std::string& path) {
	//Move the pak to the copy source
	fs::path delSourcePath (path);
	delSourcePath += ".delSource";

#ifdef PLATFORM_WINDOWS
	std::error_code err;
	fs::rename (fs::path (path), delSourcePath, err);
	if (err) {
		return false;
	}
#elif defined(__APPLE__)
	boost::system::error_code err;
	try {
		fs::rename (fs::path (path), delSourcePath);
	} catch (...) {
		return false;
	}
#else
#	error "OS not implemented!"
#endif

	//Create empty pak
	if (!CreateEmpty (path)) {
		return false;
	}

	//Open old pak and read header
	std::fstream stream;
	Header header;
	if (!OpenPakAndReadHeader (delSourcePath.string (), stream, header)) {
		return false;
	}

	//Enumerate directory and collect remaining files to the new pak
	stream.seekg (header.offset);
	if (!stream) {
		return false;
	}

	std::vector<std::string> filesToCopy;
	std::map<std::string, FileEntry> entries;
	for (uint32_t i = 0, iEnd = header.size / sizeof (FileEntry); i < iEnd; ++i) {
		FileEntry entry;
		stream >> entry;
		if (!stream) {
			return false;
		}

		std::string name (entry.path);
		bool haveToDelete = name.length () >= 3 && name[0] == '*' && name[1] == '*' && name[2] == '*'; //The file has been marked for deletion
		if (!haveToDelete) {
			filesToCopy.push_back (name);
			entries.emplace (name, entry);
		}
	}

	//Write files to the new pak
	if (!AddFiles (path, filesToCopy, [&stream, &entries] (const std::string& name, std::ostream& writer) -> bool {
		auto it = entries.find (name);
		if (it == entries.end ()) {
			return false;
		}

		const FileEntry& entry = it->second;
		stream.seekg (entry.offset);
		if (!stream) {
			return false;
		}

		const uint64_t chunkSize = 1024 * 1024;
		uint64_t chunkCount = entry.size / chunkSize;
		if (entry.size % chunkSize > 0) {
			++chunkCount;
		}

		std::vector<uint8_t> buffer (chunkSize);
		while (chunkCount--) {
			uint64_t readSize = chunkCount == 0 ? entry.size % chunkSize : chunkSize;
			if (readSize <= 0) {
				continue;
			}

			stream.read ((char*) &buffer[0], readSize);
			if (!stream) {
				return false;
			}

			writer.write ((const char*) &buffer[0], readSize);
			if (!writer) {
				return false;
			}
		}

		return true;
	})) {
		return false;
	}

	//Delete old pak
	stream.close ();
	fs::remove (delSourcePath, err);
	if (err) {
		return false;
	}

	return true;
}
