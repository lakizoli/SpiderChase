#pragma once

#include <assimp/material.h>
#include <glm/detail/type_vec4.hpp>

//Assimp material usage: http://assimp.sourceforge.net/lib_html/materials.html
//We support only one texture in each slot at this time!
//We use not all properties of the material!

class Material {
	struct TextureInfo {
		std::string name;
		uint32_t uvChannel;
		float blend;

		TextureInfo () : uvChannel ((uint32_t)-1), blend (1.0f) {}
	};

	enum class BlendingModes {
		Unknown,

		SrcAlpha_DstOneMinusAlpha,
		SrcAlpha_DstAlpha,
	};

	std::string _name;

	glm::vec4 _ambientColor;
	glm::vec4 _diffuseColor;
	glm::vec4 _specularColor;
	glm::vec4 _emissiveColor;
	glm::vec4 _transparentColor;

	bool _isTwoSided;
	BlendingModes _blendingMode;
	float _opacity;
	float _shininess;
	float _shininess_strength;

	std::shared_ptr<TextureInfo> _ambientMap;
	std::shared_ptr<TextureInfo> _diffuseMap;
	std::shared_ptr<TextureInfo> _normalMap;
	std::shared_ptr<TextureInfo> _lightMap;

	static std::string ReadStringProperty (const aiMaterial* colladaMaterial, const char* key, uint32_t type, uint32_t idx, const std::string& defaultValue = std::string ());
	static glm::vec4 ReadColorProperty (const aiMaterial* colladaMaterial, const char* key, uint32_t type, uint32_t idx, const glm::vec4& defaultValue = { 0, 0, 0, 0 });
	static float ReadFloatProperty (const aiMaterial* colladaMaterial, const char* key, uint32_t type, uint32_t idx, float defaultValue = 0.0f);
	static uint32_t ReadUIntProperty (const aiMaterial* colladaMaterial, const char* key, uint32_t type, uint32_t idx, uint32_t defaultValue = 0);
	static std::shared_ptr<TextureInfo> ReadTextureInfo (aiTextureType textureType, const aiMaterial* colladaMaterial);

public:
	Material (const aiMaterial* colladaMaterial);
};
