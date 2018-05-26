#include "stdafx.h"
#include "imagereader.hpp"

#ifdef _WINDOWS

#include <Shlwapi.h>
#include <atlbase.h>

using namespace Gdiplus;

ImageReader::ImageReader () :
	_gdiplusToken (0)
{
	GdiplusStartup (&_gdiplusToken, &_gdiplusStartupInput, nullptr);
}

ImageReader::~ImageReader () {
	GdiplusShutdown (_gdiplusToken);
	_gdiplusToken = 0;
}

bool ImageReader::ReadImage (std::istream& stream, uint64_t len) const {
	std::vector<uint8_t> data;
	data.resize (len);

	stream.read ((char*)&data[0], len);
	if (!stream) {
		return false;
	}

	CComPtr<IStream> memStream;
	memStream.Attach (SHCreateMemStream (&data[0], (UINT)data.size ()));
	if (memStream == nullptr) {
		return false;
	}

	std::shared_ptr<Image> img (Image::FromStream (memStream));
	if (img == nullptr) {
		return false;
	}

	if (img->GetLastStatus () != Ok) {
		return false;
	}

	//TODO: ...

	return true;
}

#else //_WINDOWS
#	error OS not implemented!
#endif //_WINDOWS
