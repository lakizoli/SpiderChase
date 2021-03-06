#include "stdafx.h"
#include "Platform.hpp"

#include "EglContext.h"
#include <windows.h>
#include <wincodec.h>
#include <atlbase.h>

#include <filesystem>
namespace fs = std::experimental::filesystem;

namespace Platform {

class TechnologyIniter {
public:
	TechnologyIniter () {
		CoInitialize (nullptr);
	}

	~TechnologyIniter () {
		CoUninitialize ();
	}
} g_technologyIniter;

std::string PathForResource (const std::string& resourceName, const std::string& resourceExtension) {
	//On windows all resources have to be placed besides of the game executable!
	//So we have to return only a relative path for it...
	return resourceName + "." + resourceExtension;
}

bool ReadPixels (const std::vector<uint8_t>& imageData, uint32_t& width, uint32_t& height, uint32_t& channelCount, std::vector<uint8_t>& pixels) {
	CComPtr<IWICImagingFactory> wicImagingFactory;
	if (FAILED (wicImagingFactory.CoCreateInstance (CLSID_WICImagingFactory))) {
		return false;
	}

	CComPtr<IWICStream> wicStream;
	if (FAILED (wicImagingFactory->CreateStream (&wicStream))) {
		return false;
	}

	if (FAILED (wicStream->InitializeFromMemory ((WICInProcPointer)&imageData[0], (DWORD)imageData.size ()))) {
		return false;
	}

	CComPtr<IWICBitmapDecoder> wicDecoder;
	if (FAILED (wicImagingFactory->CreateDecoderFromStream (wicStream, nullptr, WICDecodeMetadataCacheOnLoad, &wicDecoder))) {
		return false;
	}

	uint32_t frameCount = 0;
	if (FAILED (wicDecoder->GetFrameCount (&frameCount))) {
		return false;
	}

	if (frameCount < 1) {
		return false;
	}

	CComPtr<IWICBitmapFrameDecode> wicFrame;
	if (FAILED (wicDecoder->GetFrame (0, &wicFrame))) {
		return false;
	}

	if (FAILED (wicFrame->GetSize (&width, &height))) {
		return false;
	}

	WICPixelFormatGUID pixelFormatID;
	if (FAILED (wicFrame->GetPixelFormat (&pixelFormatID))) {
		return false;
	}

	CComPtr<IWICComponentInfo> componentInfo;
	if (FAILED (wicImagingFactory->CreateComponentInfo (pixelFormatID, &componentInfo))) {
		return false;
	}

	CComPtr<IWICPixelFormatInfo> pixelFormatInfo;
	if (FAILED (componentInfo->QueryInterface (IID_IWICPixelFormatInfo, (LPVOID*)&pixelFormatInfo))) {
		return false;
	}

	if (FAILED (pixelFormatInfo->GetChannelCount (&channelCount))) {
		return false;
	}

	uint32_t stride;
	WICPixelFormatGUID desiredWicPixelFormat;
	if (channelCount == 1) { //ALPHA only
		desiredWicPixelFormat = GUID_WICPixelFormat8bppGray;
		stride = 4 * ((width + 3) / 4);
	} else if (channelCount == 3) { //RGB
		desiredWicPixelFormat = GUID_WICPixelFormat24bppRGB;
		stride = 4 * ((width * 3 + 3) / 4);
	} else { //RGBA
		desiredWicPixelFormat = GUID_WICPixelFormat32bppRGBA;
		stride = 4 * width;
	}

	CComPtr<IWICBitmapFlipRotator> wicRotator;
	if (FAILED (wicImagingFactory->CreateBitmapFlipRotator (&wicRotator))) { //Create flip/rotator
		return false;
	}

	//Have to flip the image vertically because of difference in blender's UV system (+Y upside down) and OpenGL's UV system (+Y bottom up)!
	if (FAILED (wicRotator->Initialize (wicFrame, WICBitmapTransformFlipVertical))) {
		return false;
	}

	CComPtr<IWICFormatConverter> wicConverter;
	if (FAILED (wicImagingFactory->CreateFormatConverter (&wicConverter))) {
		return false;
	}

	if (FAILED (wicConverter->Initialize (wicRotator, desiredWicPixelFormat, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeCustom))) {
		return false;
	}

	pixels.resize (height * stride);
	if (FAILED (wicConverter->CopyPixels (nullptr, stride, (uint32_t)pixels.size (), &pixels[0]))) {
		return false;
	}

	return true;
}

std::string FileNameFromPath (const std::string& path) {
	fs::path fsPath (path);
	return fsPath.filename ().string ();
}

std::string ExtensionFromPath (const std::string& path) {
	fs::path fsPath (path);
	return fsPath.extension ().string ();
}

std::string ParentOfPath (const std::string& path) {
	fs::path fsPath (path);
	return fsPath.parent_path ().string ();
}

bool RenameFile (const std::string& srcPath, const std::string& destPath) {
	std::error_code err;
	fs::rename (fs::path (srcPath), fs::path (destPath), err);
	if (err) {
		return false;
	}

	return true;
}

bool RemoveFile (const std::string& path) {
	std::error_code err;
	fs::remove (fs::path (path), err);
	if (err) {
		return false;
	}

	return false;
}

} //namespace Platform
