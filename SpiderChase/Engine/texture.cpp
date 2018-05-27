#include "stdafx.h"
#include "texture.hpp"

Texture::Texture (const std::string& name, PixelFormat pixelFormat, uint32_t width, uint32_t height, const std::vector<uint8_t>& data) :
	_name (name),
	_pixelFormat (pixelFormat),
	_width (width),
	_height (height)
{
	gl::GenTextures (1, &_tex);
	gl::BindTexture (GL_TEXTURE_2D, _tex);

	switch (pixelFormat) {
	case PixelFormat::RGB_888:
		gl::TexImage2D (GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, &data[0]); //Have to use 4 byte alignment!
		break;
	case PixelFormat::BGRA_8888:
		gl::TexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, &data[0]);
		break;
	default:
		break;
	}

	gl::GenerateMipmap (GL_TEXTURE_2D);

	gl::TexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	gl::TexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	gl::TexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gl::TexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::Update (double frameTime) {
	//...Nothing to do...
}

void Texture::Render () const {
	gl::BindTexture (GL_TEXTURE_2D, _tex);
}

void Texture::Release () {
	gl::DeleteTextures (1, &_tex);
	_tex = 0;
}
