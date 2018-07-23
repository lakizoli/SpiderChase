#pragma once

//TODO: implement writing -> https://github.com/id-Software/Quake-Tools/blob/master/qutils/QFILES/QFILES.C
//TODO: implement packing workflow to the game

class Pak {
public:
	struct Header {
		char mimeID[4];
		uint32_t offset;
		uint32_t size;

		Header () : mimeID { 0,0,0,0 }, offset (0), size (0) {}
	};

	struct FileEntry {
		char path[56];
		uint32_t offset;
		uint32_t size;

		FileEntry () : offset (0), size (0) {
			memset (path, 0, sizeof (path));
		}
	};

private:
	std::string _path;
	std::fstream _stream;
	Header _header;

	Pak () {}

//Read interface
public:
	static std::shared_ptr<Pak> OpenForRead (const std::string& path);

	bool IsValid () const;
	bool ReadDirectory (std::vector<FileEntry>& dir);
	bool ReadFile (const FileEntry& fileEntry, std::function<void (std::istream&)> callback);

//Write interface
public:
	static bool CreateEmpty (const std::string& path);
	static bool AddFiles (const std::string& path, const std::vector<std::string>& names, std::function<void (const std::string&, std::ostream&)> writer);
	static bool MarkFilesToDelete (const std::string& path, const std::set<std::string>& names);
	static bool DeleteMarkedFiles (const std::string& path);
};
