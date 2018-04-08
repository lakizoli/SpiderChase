#pragma once

//TODO: implement writing -> https://github.com/id-Software/Quake-Tools/blob/master/qutils/QFILES/QFILES.C
//TODO: implement packing workflow to the game

class Pak {
public:
	struct Header {
		char mimeID[4];
		uint32_t offset;
		uint32_t size;

		Header ();
	};

	struct FileEntry {
		char path[56];
		uint32_t offset;
		uint32_t size;

		FileEntry ();
	};

private:
	std::string _path;
	std::fstream _stream;
	Header _header;

public:
	Pak (const std::string& path);
	~Pak ();

	bool IsValid () const;

	void EnumerateDirectory (std::function<bool (const FileEntry&)> callback);
	bool ReadFile (const std::string& path, std::function<bool (const std::string&, std::istream&)> callback);
};
