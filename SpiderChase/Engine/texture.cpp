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
		gl::TexImage2D (GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, &data[0]); //Have to use 4 byte alignment!
		break;
	case PixelFormat::RGBA_8888:
		gl::TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
		break;
	case PixelFormat::ALPHA_8:
		gl::TexImage2D (GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, &data[0]); //Have to use 4 byte alignment!
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
	if (_tex > 0) {
		gl::DeleteTextures (1, &_tex);
		_tex = 0;
	}
}
