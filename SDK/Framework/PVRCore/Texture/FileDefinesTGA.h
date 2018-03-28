/*!
\brief Defines used internally by the TGA reader.
\file PVRCore/Texture/FileDefinesTGA.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/CoreIncludes.h"
//!\cond NO_DOXYGEN
namespace pvr {
namespace texture_tga {
struct FileHeader
{
	uint8_t identSize;          // size of ID field that follows 18 char header (0 usually)
	uint8_t colorMapType;      // type of color map 0=none, 1=has palette
	uint8_t imageType;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed
	int16_t colorMapStart;     // first color map entry in palette
	int16_t colorMapLength;    // number of colors in palette
	uint8_t colorMapBits;      // number of bits per palette entry 15,16,24,32
	int16_t xStart;             // image x origin
	int16_t yStart;             // image y origin
	int16_t width;              // image width in pixels
	int16_t height;             // image height in pixels
	uint8_t bits;               // image bits per pixel 8,15,16,24,32
	uint8_t descriptor;         // image descriptor bits (vh flip bits)
};

namespace ColorMap {
enum Enum
{
	None,
	Paletted
};
};

namespace ImageType {
enum Enum
{
	None,
	Indexed,
	RGB,
	GreyScale,
	RunLengthNone = 8,
	RunLengthIndexed,
	RunLengthRGB,
	RunLengthGreyScale,
	RunLengthHuffmanDelta = 32,
	RunLengthHuffmanDeltaFourPassQuadTree
};
};

enum DescriptorFlag
{
	DescriptorFlagAlpha = 8
};

// Expected size of a header in file
static const uint32_t ExpectedHeaderSize = 18;

}
}
//!\endcond