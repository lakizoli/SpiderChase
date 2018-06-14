//
//  Platform.hpp
//  SpiderChase
//
//  Created by Laki, Zoltan on 2018. 06. 14..
//  Copyright © 2018. Immortal Games. All rights reserved.
//

std::string PathForResource (const std::string& resourceName, const std::string& resourceExtension);
bool ReadImage (const std::vector<uint8_t>& imageData, uint32_t& width, uint32_t& height, uint32_t& channelCount, std::vector<uint8_t>& pixels);

#ifdef __OBJC__

#import <Foundation/Foundation.h>

@interface ImageReader : NSObject

@end

#endif //__OBJC__