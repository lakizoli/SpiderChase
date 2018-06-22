//
//  Platform.mm
//  SpiderChase
//
//  Created by Laki, Zoltan on 2018. 06. 14..
//  Copyright © 2018. Immortal Games. All rights reserved.
//

#include "stdafx.h"
#import "Platform.hpp"
#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>

@interface ImageHelper : NSObject

@end

@implementation ImageHelper

//Color space detection logic is defined in documentation at url (we support only 24bit (RGB), 32bit (RGBA) and 8bit (Aplha only) bitmaps!):
//https://developer.apple.com/library/archive/documentation/GraphicsImaging/Conceptual/drawingwithquartz2d/dq_context/dq_context.html#//apple_ref/doc/uid/TP30001066-CH203-BCIBHHBB

+(UIImage*) readImageFromBuffer:(const std::vector<uint8_t>&)imageBytes {
	NSData* data = [NSData dataWithBytesNoCopy:(void*)&imageBytes[0] length:imageBytes.size () freeWhenDone:NO];
	return [UIImage imageWithData:data];
}

+(BOOL) isAlphaOnly:(size_t)bpp bpc:(size_t)bpc alphaInfo:(CGImageAlphaInfo)alphaInfo {
	return bpp == 8 && bpc == 8 && (alphaInfo == kCGImageAlphaOnly || alphaInfo == kCGImageAlphaNone);
}

+(BOOL) isRGBOnly:(size_t)bpp bpc:(size_t)bpc alphaInfo:(CGImageAlphaInfo)alphaInfo {
	return bpp == 32 && bpc == 8 && (alphaInfo == kCGImageAlphaNoneSkipFirst || alphaInfo == kCGImageAlphaNoneSkipLast);
}

+(BOOL) isRGBA:(size_t)bpp bpc:(size_t)bpc alphaInfo:(CGImageAlphaInfo)alphaInfo {
	return bpp == 32 && bpc == 8 && (alphaInfo == kCGImageAlphaPremultipliedFirst || alphaInfo == kCGImageAlphaPremultipliedLast);
}

+(void) copyRGBChannelsWithoutAlpha:(UIImage*)image destBuffer:(std::vector<uint32_t>&)destBuffer {
	CGSize imageSize = [image size];
	destBuffer.resize (4 * imageSize.width * imageSize.height);
	
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB ();
	CGContextRef context = CGBitmapContextCreate (&destBuffer[0],
												  imageSize.width,
												  imageSize.height,
												  8, //bits per component
												  4 * imageSize.width, //bytes per row
												  colorSpace, //CG RGB color space
												  kCGImageAlphaNoneSkipLast); //RRRRRRRR_GGGGGGGG_BBBBBBBB_........
	
	//Have to flip the image vertically because of difference in blender's UV system (+Y upside down) and OpenGL's UV system (+Y bottom up)!
	CGContextTranslateCTM (context, 0, imageSize.height);
	CGContextScaleCTM (context, 1.0, -1.0);

	CGContextDrawImage (context, CGRectMake (0, 0, imageSize.width, imageSize.height), [image CGImage]);
	CGContextRelease (context);
	CGColorSpaceRelease (colorSpace);
}

+(void) copyAlphaChannelOnly:(UIImage*)image destBuffer:(std::vector<uint8_t>&)destBuffer {
	CGSize imageSize = [image size];
	destBuffer.resize (1 * imageSize.width * imageSize.height);
	
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceGray ();
	CGContextRef context = CGBitmapContextCreate (&destBuffer[0],
												  imageSize.width,
												  imageSize.height,
												  8, //bits per component
												  1 * imageSize.width, //bytes per row
												  colorSpace, //CG RGB color space
												  kCGImageAlphaOnly); //AAAAAAAA
	
	//Have to flip the image vertically because of difference in blender's UV system (+Y upside down) and OpenGL's UV system (+Y bottom up)!
	CGContextTranslateCTM (context, 0, imageSize.height);
	CGContextScaleCTM (context, 1.0, -1.0);
	
	CGContextDrawImage (context, CGRectMake (0, 0, imageSize.width, imageSize.height), [image CGImage]);
	CGContextRelease (context);
	CGColorSpaceRelease (colorSpace);
}

+(void) copyBoringTestRGBA:(std::vector<uint8_t>&)destBuffer {
	destBuffer.resize (1024 * 1024 * 4);
	
	for (uint32_t i = 0; i < 1024 * 1024 * 4; i += 4) {
		uint32_t x = i % 1024;
		uint32_t y = i / 1024;
		
		if (x / 32 % 2 == 0 ^ y / 32 % 2 == 0) {
			destBuffer[i] = 0xff;
			destBuffer[i + 1] = 0x00;
			destBuffer[i + 2] = 0x00;
			destBuffer[i + 3] = 0xff;
		} else {
			destBuffer[i] = 0x00;
			destBuffer[i + 1] = 0xff;
			destBuffer[i + 2] = 0x00;
			destBuffer[i + 3] = 0xff;
		}
	}
}

@end

std::string PathForResource (const std::string& resourceName, const std::string& resourceExtension) {
	@autoreleasepool {
		NSString* resName = [NSString stringWithUTF8String:resourceName.c_str ()];
		NSString* resType = [NSString stringWithUTF8String:resourceExtension.c_str ()];
		NSString* filePath = [[NSBundle mainBundle] pathForResource:resName ofType:resType];
		return [filePath UTF8String];
	}
}

bool ReadPixels (const std::vector<uint8_t>& imageData, uint32_t& width, uint32_t& height, uint32_t& channelCount, std::vector<uint8_t>& pixels) {
	@autoreleasepool {
		UIImage* image = [ImageHelper readImageFromBuffer:imageData];
		
		CGSize imageSize = [image size];
		width = imageSize.width;
		height = imageSize.height;
		
		CGImageRef imageRef = [image CGImage];
		
		size_t imageBitsPerPixel = CGImageGetBitsPerPixel (imageRef);
		size_t imageBitPerComponent = CGImageGetBitsPerComponent (imageRef);
		CGImageAlphaInfo imageAlphaInfo = CGImageGetAlphaInfo (imageRef);
		
		if ([ImageHelper isAlphaOnly:imageBitsPerPixel bpc:imageBitPerComponent alphaInfo:imageAlphaInfo]) { //Alpha only
			[ImageHelper copyAlphaChannelOnly:image destBuffer:pixels];
		} else if ([ImageHelper isRGBOnly:imageBitsPerPixel bpc:imageBitPerComponent alphaInfo:imageAlphaInfo]) { //RGB only
			std::vector<uint32_t> rgbBuffer;
			[ImageHelper copyRGBChannelsWithoutAlpha:image destBuffer:rgbBuffer];

			uint32_t stride = 4 * ((width * 3 + 3) / 4);
			uint64_t size = height * stride;
			pixels.resize (size);
			
			for (uint32_t y = 0; y < height; ++y) {
				for (uint32_t x = 0; x < width; ++x) {
					uint32_t pixelRGB = rgbBuffer[y * width + x];
					
					uint32_t destPixelOffset = y * stride + x * 3;
					memcpy (&pixels[destPixelOffset], &pixelRGB, 3);
				}
			}
		} else if ([ImageHelper isRGBA:imageBitsPerPixel bpc:imageBitPerComponent alphaInfo:imageAlphaInfo]) { //RGBA
			std::vector<uint32_t> rgbBuffer;
			[ImageHelper copyRGBChannelsWithoutAlpha:image destBuffer:rgbBuffer];

			std::vector<uint8_t> alphaBuffer;
			[ImageHelper copyAlphaChannelOnly:image destBuffer:alphaBuffer];

			uint32_t stride = 4 * width;
			uint64_t size = height * stride;
			pixels.resize (size);
			
			for (uint32_t y = 0; y < height; ++y) {
				for (uint32_t x = 0; x < width; ++x) {
					uint32_t pixelRGB = rgbBuffer[y * width + x];
					uint32_t pixelAlpha = (uint32_t) alphaBuffer[y * width + x];
					pixels[y * stride + x * 4] = (pixelRGB & 0x00FFFFFF) | ((pixelAlpha << 24) & 0xFF000000);
				}
			}
		} else { //Unsupported image format
			return false;
		}
		
		return true;
	}
}
