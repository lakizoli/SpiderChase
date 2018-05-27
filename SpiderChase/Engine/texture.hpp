#pragma once

#include <DynamicGles.h>

class Texture {
public:
	enum class PixelFormat {
		BGRA_8888,
		RGB_888
	};

private:
	std::string _name;

	PixelFormat _pixelFormat;
	uint32_t _width;
	uint32_t _height;

	GLuint _tex; ///< Texture object id

public:
	Texture (const std::string& name, PixelFormat pixelFormat, uint32_t width, uint32_t height, const std::vector<uint8_t>& data);

	void Update (double frameTime);
	void Render () const;

	void Release ();
};
