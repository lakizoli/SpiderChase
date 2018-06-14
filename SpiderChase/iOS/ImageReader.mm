//
//  ImageReader.m
//  SpiderChase
//
//  Created by Laki, Zoltan on 2018. 06. 14..
//  Copyright © 2018. Immortal Games. All rights reserved.
//

#include "stdafx.h"
#import "ImageReader.hpp"
#import <UIKit/UIKit.h>

@implementation ImageReader

+(UIImage*)readImageFromBuffer:(const std::vector<uint8_t>&)imageBytes {
	NSData* data = [NSData dataWithBytesNoCopy:(void*)&imageBytes[0] length:imageBytes.size () freeWhenDone:NO];
	return [UIImage imageWithData:data];
}

@end

bool ReadImage (const std::vector<uint8_t>& imageData, uint32_t& width, uint32_t& height, uint32_t& channelCount, std::vector<uint8_t>& pixels) {
	@autoreleasepool {
		UIImage* image = [ImageReader readImageFromBuffer:imageData];
		
		CGSize imageSize = [image size];
		width = imageSize.width;
		height = imageSize.height;
		
		CGImageRef imageRef = [image CGImage];

		CGImageAlphaInfo alphaInfo = CGImageGetAlphaInfo (imageRef);
		CGImageAlphaInfo destAlphaInfo = kCGImageAlphaNone;
		uint32_t stride = 0;
		switch (alphaInfo) {
			case kCGImageAlphaNone: //RGB
				destAlphaInfo = kCGImageAlphaNone;
				channelCount = 3;
				stride = 4 * ((width * 3 + 3) / 4);
				break;
			case kCGImageAlphaOnly: //Aplha only
				destAlphaInfo = kCGImageAlphaOnly;
				channelCount = 1;
				stride = 4 * ((width + 3) / 4);
				break;
			default: //All RGBA format variants
				destAlphaInfo = kCGImageAlphaLast;
				channelCount = 4;
				stride = 4 * width;
				break;
		}

		// Alloc data that the image data will be put into
		pixels.resize (height * stride * channelCount);
		
		// Setup color space
		CGColorSpaceRef colorSpace = channelCount == 1 ? CGColorSpaceCreateDeviceGray () : CGColorSpaceCreateDeviceRGB ();
		
		// Create a CGBitmapContext to draw an image into
		NSUInteger bitsPerComponent = 8;
		CGContextRef context = CGBitmapContextCreate (&pixels[0], width, height, bitsPerComponent, stride, colorSpace, destAlphaInfo | kCGBitmapByteOrder32Big);
		CGColorSpaceRelease (colorSpace);
		
		// Draw the image which will populate rawData
		CGContextDrawImage (context, CGRectMake (0, 0, width, height), imageRef);
		CGContextRelease (context);
		
		CGImageRelease (imageRef);
		
		return false;
	}
}
