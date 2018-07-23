#pragma once

namespace Platform {

std::string PathForResource (const std::string& resourceName, const std::string& resourceExtension);
bool ReadPixels (const std::vector<uint8_t>& imageData, uint32_t& width, uint32_t& height, uint32_t& channelCount, std::vector<uint8_t>& pixels);

std::string FileNameFromPath (const std::string& path);
std::string ExtensionFromPath (const std::string& path);
std::string ParentOfPath (const std::string& path);
bool RenameFile (const std::string& srcPath, const std::string& destPath);
bool RemoveFile (const std::string& path);

} //namespace Platform
