#pragma once

std::string PathForResource (const std::string& resourceName, const std::string& resourceExtension);
bool ReadPixels (const std::vector<uint8_t>& imageData, uint32_t& width, uint32_t& height, uint32_t& channelCount, std::vector<uint8_t>& pixels);
