#include "stdafx.h"
#include "material.hpp"

std::string Material::ReadStringProperty (const aiMaterial* colladaMaterial, const char* key, uint32_t type, uint32_t idx, const std::string& defaultValue) {
	aiString str;
	if (colladaMaterial->Get (key, type, idx, str) != aiReturn_SUCCESS) {
		return defaultValue;
	}

	return str.C_Str ();
}

glm::vec4 Material::ReadColorProperty (const aiMaterial* colladaMaterial, const char* key, uint32_t type, uint32_t idx, const glm::vec4& defaultValue) {
	aiColor3D col;
	if (colladaMaterial->Get (key, type, idx, col) != aiReturn_SUCCESS) {
		return defaultValue;
	}

	return glm::vec4 (col.r, col.g, col.b, 1.0f);
}

float Material::ReadFloatProperty (const aiMaterial* colladaMaterial, const char* key, uint32_t type, uint32_t idx, float defaultValue) {
	float val = defaultValue;
	if (colladaMaterial->Get (key, type, idx, val) != aiReturn_SUCCESS) {
		return defaultValue;
	}

	return val;
}

uint32_t Material::ReadUIntProperty (const aiMaterial* colladaMaterial, const char* key, uint32_t type, uint32_t idx, uint32_t defaultValue) {
	int32_t val = (int32_t) defaultValue;
	if (colladaMaterial->Get (key, type, idx, val) != aiReturn_SUCCESS) {
		return defaultValue;
	}

	return (uint32_t) val;
}

std::shared_ptr<Material::TextureInfo> Material::ReadTextureInfo (aiTextureType textureType, const aiMaterial* colladaMaterial, const std::map<std::string, std::shared_ptr<Texture>>& textures) {
	if (colladaMaterial->GetTextureCount (textureType) > 0) {
		aiString path;
		aiTextureMapping mapping = aiTextureMapping_UV;
		uint32_t uvIndex = 0;
		float blend = 1.0f;
		//aiTextureOp operation = aiTextureOp_Add;
		//aiTextureMapMode mapMode = aiTextureMapMode_Wrap;

		aiReturn resCode = colladaMaterial->GetTexture (textureType, 0, &path, &mapping, &uvIndex, &blend /*, &operation, &mapMode*/);
		if (resCode == aiReturn_SUCCESS && mapping == aiTextureMapping_UV) {
			std::string texName = path.C_Str ();
			std::transform (texName.begin (), texName.end (), texName.begin (), ::tolower);

			std::shared_ptr<TextureInfo> info (std::make_shared<TextureInfo> ());
			info->name = texName;
			info->uvChannel = uvIndex;
			info->blend = blend;

			auto it = textures.find (texName);
			if (it != textures.end ()) {
				info->texture = it->second;
				return info;
			}
		}
	}

	return nullptr;
}

Material::Material (const aiMaterial* colladaMaterial, const std::map<std::string, std::shared_ptr<Texture>>& textures) {
	//Read texture maps
	_ambientMap = ReadTextureInfo (aiTextureType_AMBIENT, colladaMaterial, textures);
	_diffuseMap = ReadTextureInfo (aiTextureType_DIFFUSE, colladaMaterial, textures);
	_normalMap = ReadTextureInfo (aiTextureType_NORMALS, colladaMaterial, textures);
	_lightMap = ReadTextureInfo (aiTextureType_LIGHTMAP, colladaMaterial, textures);

	//Read material properties
	_ambientColor = ReadColorProperty (colladaMaterial, AI_MATKEY_COLOR_AMBIENT);
	_diffuseColor = ReadColorProperty (colladaMaterial, AI_MATKEY_COLOR_DIFFUSE);
	_specularColor = ReadColorProperty (colladaMaterial, AI_MATKEY_COLOR_SPECULAR);
	_emissiveColor = ReadColorProperty (colladaMaterial, AI_MATKEY_COLOR_EMISSIVE);
	_transparentColor = ReadColorProperty (colladaMaterial, AI_MATKEY_COLOR_TRANSPARENT);

	_isTwoSided = ReadUIntProperty (colladaMaterial, AI_MATKEY_TWOSIDED) != 0;

	switch (ReadUIntProperty (colladaMaterial, AI_MATKEY_BLEND_FUNC)) {
	case aiBlendMode_Additive:
		_blendingMode = BlendingModes::SrcAlpha_DstAlpha;
		break;
	default:
	case aiBlendMode_Default:
		_blendingMode = BlendingModes::SrcAlpha_DstOneMinusAlpha;
		break;
	}

	_opacity = ReadFloatProperty (colladaMaterial, AI_MATKEY_OPACITY, 1.0f);
	_shininess = ReadFloatProperty (colladaMaterial, AI_MATKEY_SHININESS);
	_shininess_strength = ReadFloatProperty (colladaMaterial, AI_MATKEY_SHININESS_STRENGTH, 1.0f);
}
