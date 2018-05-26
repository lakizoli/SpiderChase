#pragma once

#ifdef _WINDOWS
#	include <windows.h>
#	include <gdiplus.h>
#else //_WINDOWS
#	error OS not implemented!
#endif //_WINDOWS

class ImageReader {
#ifdef _WINDOWS
	Gdiplus::GdiplusStartupInput _gdiplusStartupInput;
	ULONG_PTR _gdiplusToken;
#else //_WINDOWS
#	error OS not implemented!
#endif //_WINDOWS

	ImageReader ();
	~ImageReader ();

public:
	static ImageReader& Get () {
		static ImageReader inst;
		return inst;
	}

	bool ReadImage (std::istream& stream, uint64_t len) const;
};
