#include "stdafx.h"
#include "scene.hpp"
#include "pak.hpp"

#ifdef _WINDOWS
#	include <windows.h>
#	include <gdiplus.h>
#	include <Shlwapi.h>
#	include <atlbase.h>
#endif //_WINDOWS

#include "EglContext.h"
#include "input.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#ifdef _WINDOWS

class TechnologyIniter {
	Gdiplus::GdiplusStartupInput _gdiplusStartupInput;
	ULONG_PTR _gdiplusToken;
	
public:
	TechnologyIniter () {
		GdiplusStartup (&_gdiplusToken, &_gdiplusStartupInput, nullptr);
	}

	~TechnologyIniter () {
		Gdiplus::GdiplusShutdown (_gdiplusToken);
		_gdiplusToken = 0;
	}
} g_technologyIniter;

#else //_WINDOWS
#	error OS not implemented!
#endif //_WINDOWS

std::map<std::string, Scene::SceneCreator>& Scene::GetCreatorMap () {
	static std::map<std::string, SceneCreator> creators;
	return creators;
}

Scene::Scene () :
	_isInited (false)
{
}

uint32_t Scene::RegisterSceneCreator (const std::string& sceneID, SceneCreator sceneCreator) {
	std::map<std::string, SceneCreator>& creators = GetCreatorMap ();
	creators.emplace (sceneID, sceneCreator);
	return 42;
}

std::shared_ptr<Scene> Scene::Create (const std::string& sceneID) {
	const std::map<std::string, SceneCreator>& creators = GetCreatorMap ();
	auto it = creators.find (sceneID);
	if (it == creators.end ()) {
		return nullptr;
	}

	return it->second ();
}

Scene::SceneResults Scene::Step (double currentTimeInSec, const InputState& _inputState) {
	if (!_isInited) {
		Init ();
		_isInited = true;
	}

	SceneResults updateRes = Update (currentTimeInSec, _inputState);
	if (updateRes != SceneResults::Continue) { //Have to exit scene...
		Release ();

		_isInited = false;

		return updateRes;
	}

	Render ();

	return SceneResults::Continue;
}

std::tuple<bool, uint32_t> Scene::CompileShader (uint32_t type, const char* source) {
	// Create the fragment shader object
	uint32_t shaderID = gl::CreateShader ((GLenum) type);

	// Load the source code into it
	gl::ShaderSource (shaderID, 1, (const char**) &source, NULL);

	// Compile the source code
	gl::CompileShader (shaderID);

	// Check if compilation succeeded
	int32_t bShaderCompiled = 0;
	gl::GetShaderiv (shaderID, GL_COMPILE_STATUS, &bShaderCompiled);
	if (!bShaderCompiled) {
		// An error happened, first retrieve the length of the log message
		int32_t infoLogLength, numCharsWritten;
		gl::GetShaderiv (shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

		// Allocate enough space for the message and retrieve it
		std::vector<char> infoLog (infoLogLength);
		gl::GetShaderInfoLog (shaderID, infoLogLength, &numCharsWritten, infoLog.data ());

		//  Displays the message in a dialog box when the application quits
		//  using the shell PVRShellSet function with first parameter prefExitMessage.
		std::string message ("Failed to compile fragment shader: ");
		message += infoLog.data ();

		Log (LogLevel::Error, "Shader compile error: %s", message.c_str ());
		return std::make_tuple<bool, uint32_t> (false, 0);
	}

	return std::make_tuple (true, shaderID);
}

std::tuple<bool, uint32_t> Scene::LinkProgram (const std::string& name, const std::vector<uint32_t>& shaderIDs, ShaderBindCallback bindCallback) {
	// Create the shader program
	uint32_t programID = gl::CreateProgram ();

	// Attach the fragment and vertex shaders to it
	for (uint32_t shaderID : shaderIDs) {
		gl::AttachShader (programID, shaderID);
	}

	// Bind the custom vertex attribute "myVertex" to location VERTEX_ARRAY
	if (bindCallback) {
		bindCallback (name, programID);
	}

	// Link the program
	gl::LinkProgram (programID);

	// Check if linking succeeded in the same way we checked for compilation success
	int32_t bLinked;
	gl::GetProgramiv (programID, GL_LINK_STATUS, &bLinked);
	if (!bLinked) {
		int32_t infoLogLength, numCharsWritten;
		gl::GetProgramiv (programID, GL_INFO_LOG_LENGTH, &infoLogLength);

		std::vector<char> infoLog (infoLogLength);
		gl::GetProgramInfoLog (programID, infoLogLength, &numCharsWritten, infoLog.data ());

		std::string message ("Failed to link program: ");
		message += infoLog.data ();

		Log (LogLevel::Error, "Shader link error: %s", message.c_str ());
		return std::make_tuple<bool, uint32_t> (false, 0);
	}

	return std::make_tuple (true, programID);
}

bool Scene::LoadShader (const std::string& shaderName, uint32_t shaderType, std::istream& stream, uint64_t len, std::shared_ptr<Assets> assets) {
	std::string source;
	source.resize (len);

	stream.read ((char*) &source[0], len);
	if (!stream) {
		return false;
	}

	auto compileRes = CompileShader (shaderType, source.c_str ());
	if (!std::get<0> (compileRes)) {
		return false;
	}

	if (shaderType == GL_VERTEX_SHADER) { //vertex shader
		assets->vertexShaders.emplace (shaderName, std::get<1> (compileRes));
	} else { //fragment shader
		assets->fragmentShaders.emplace (shaderName, std::get<1> (compileRes));
	}

	return true;
}

bool Scene::LoadProgram (const std::string& programName, std::istream& stream, uint64_t len, std::vector<std::string>& vertexShaders, std::vector<std::string>& fragmentShaders) {
	std::string source;
	source.resize (len);

	stream.read ((char*) &source[0], len);
	if (!stream) {
		return false;
	}

	std::stringstream prg (source);
	for (std::string line; std::getline (prg, line);) {
		std::string fileName = Trim (line);
		std::transform (fileName.begin (), fileName.end (), fileName.begin (), ::tolower);

		if (StringEndsWith (fileName, ".frag")) {
			fragmentShaders.push_back (fileName);
		} else if (StringEndsWith (fileName, ".vert")) {
			vertexShaders.push_back (fileName);
		}
	}

	return true;
}

std::shared_ptr<aiScene> Scene::LoadCollada (std::istream& stream, uint64_t len) {
	std::vector<uint8_t> data;
	data.resize (len);

	stream.read ((char*) &data[0], len);
	if (!stream) {
		return nullptr;
	}

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFileFromMemory (&data[0], data.size (),
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (!scene) {
		return nullptr;
	}

	return std::shared_ptr<aiScene> (importer.GetOrphanedScene ());
}

std::shared_ptr<Texture> Scene::LoadTexture (const std::string& name, std::istream& stream, uint64_t len) {
	std::vector<uint8_t> data;
	data.resize (len);

	stream.read ((char*) &data[0], len);
	if (!stream) {
		return nullptr;
	}

	std::shared_ptr<Texture> result;

#ifdef _WINDOWS

	CComPtr<IStream> memStream;
	memStream.Attach (SHCreateMemStream (&data[0], (UINT)data.size ()));
	if (memStream == nullptr) {
		return nullptr;
	}

	std::shared_ptr<Gdiplus::Bitmap> img (Gdiplus::Bitmap::FromStream (memStream));
	if (img == nullptr) {
		return nullptr;
	}

	if (img->GetLastStatus () != Gdiplus::Ok) {
		return nullptr;
	}

	uint32_t width = img->GetWidth ();
	uint32_t height = img->GetHeight ();

	bool hasAlpha = (img->GetPixelFormat () & PixelFormatAlpha) == PixelFormatAlpha;
	hasAlpha = false; //TODO: 4 byte textures not working
	Texture::PixelFormat texPixelFormat = hasAlpha ? Texture::PixelFormat::BGRA_8888 : Texture::PixelFormat::RGB_888;
	uint32_t stride = hasAlpha ? 4 * width : 4 * ((width * 3 + 3) / 4);
	std::vector<uint8_t> texData (height * stride);

	Gdiplus::Rect frame (0, 0, width, height);
	Gdiplus::BitmapData bitmapData;
	bitmapData.Scan0 = &texData[0];
	bitmapData.Width = width;
	bitmapData.Height = height;
	bitmapData.PixelFormat = hasAlpha ? PixelFormat32bppARGB : PixelFormat24bppRGB;
	bitmapData.Stride = stride;
	if (img->LockBits (&frame, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeUserInputBuf, bitmapData.PixelFormat, &bitmapData) != Gdiplus::Ok) {
		return nullptr;
	}

	//FILE* fff = fopen ("c:\\temp\\juhu1", "wb");
	//if (fff) {
	//	fwrite (&texData[0], sizeof (uint8_t), texData.size (), fff);
	//	fclose (fff);
	//}

	if (img->UnlockBits (&bitmapData) != Gdiplus::Ok) {
		return nullptr;
	}

	if (hasAlpha) { //Need to convert BGRA from ARGB
		for (size_t i = 0, iEnd = texData.size () / 4; i < iEnd; ++i) {
			uint32_t pixValue = *(uint32_t*)&texData[i * 4];
			texData[i * 4 + 0] = (uint8_t)(pixValue >> 24 & 0xff);
			texData[i * 4 + 1] = (uint8_t)(pixValue >> 16 & 0xff);
			texData[i * 4 + 2] = (uint8_t)(pixValue >> 8 & 0xff);
			texData[i * 4 + 3] = (uint8_t)(pixValue & 0xff);
		}
	}

	//fff = fopen ("c:\\temp\\juhu2", "wb");
	//if (fff) {
	//	fwrite (&texData[0], sizeof (uint8_t), texData.size (), fff);
	//	fclose (fff);
	//}


	////TODO: I think this will be a boring texture :D
	////I think the loading is wrong because full /0 char
	//texData.resize(1024 * 1024 * 3);
	//for (int i = 0; i < 1024 * 1024 * 3; i += 3) {
	//	int x = i % 1024;
	//	int y = i / 1024;

	//	if (x / 32 % 2 == 0 ^ y / 32 % 2 == 0) {
	//		texData[i] = 0xff;
	//		texData[i + 1] = 0x00;
	//		texData[i + 2] = 0x00;

	//	}
	//	else {
	//		texData[i] = 0x00;
	//		texData[i + 1] = 0xff;
	//		texData[i + 2] = 0x00;

	//	}
	//}

	result = std::make_shared<Texture> (name, texPixelFormat, width, height, texData);

#else //_WINDOWS
#	error OS not implemented!
#endif //_WINDOWS

	return result;
}

std::shared_ptr<Scene::Assets> Scene::LoadPak (const std::string& name, const SceneMaterialShaders& sceneMaterialShaders, ShaderBindCallback shaderBindCallback) {
	Log (LogLevel::Information, "*** loading pak file (%s.pak) ***", name.c_str ());

	std::shared_ptr<Pak> pak = Pak::OpenForRead (name + ".pak");
	if (pak == nullptr) {
		Log (LogLevel::Error, "Pak not found!");
		return nullptr;
	}

	std::vector<Pak::FileEntry> entries;
	if (!pak->ReadDirectory (entries)) {
		Log (LogLevel::Error, "Cannot read directory from pak!");
		return nullptr;
	}

	//Load assets
	std::shared_ptr<Assets> assets (std::make_shared<Assets> ());

	std::set<std::string> programs;
	std::map<std::string, std::vector<std::string>> programVertexShaders;
	std::map<std::string, std::vector<std::string>> programFragmentShaders;

	struct ColladaSceneInfo {
		std::string name;
		std::shared_ptr<aiScene> scene;
		std::vector<std::shared_ptr<Material>> materials;
		std::map<std::string, std::shared_ptr<Texture>> textures;
	};

	std::map<std::string, std::shared_ptr<ColladaSceneInfo>> scenes;

	for (const Pak::FileEntry& entry : entries) {
		fs::path entryPath (entry.path);
		Log (LogLevel::Information, "Load: %s", entryPath.string ().c_str ());

		std::string ext = entryPath.extension ().string ();
		if (ext == ".program") {
			if (!pak->ReadFile (entry, [&entryPath, &entry, &programs, &programVertexShaders, &programFragmentShaders] (std::istream& stream) -> bool {
				std::vector<std::string> vertexShaders;
				std::vector<std::string> fragmentShaders;
				if (!LoadProgram (entryPath.filename ().string (), stream, entry.size, vertexShaders, fragmentShaders)) {
					return false;
				}

				std::string programName = entryPath.filename ().string ();
				programs.insert (programName);
				programVertexShaders.emplace (programName, vertexShaders);
				programFragmentShaders.emplace (programName, fragmentShaders);
				return true;
			})) {
				Log (LogLevel::Error, "Cannot load program: %s from %s pak!", entryPath.filename ().string ().c_str (), name.c_str ());
				return nullptr;
			}
		} else if (ext == ".frag" || ext == ".vert") {
			uint32_t shaderType = ext == ".vert" ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
			if (!pak->ReadFile (entry, [&entryPath, &entry, shaderType, assets] (std::istream& stream) -> bool {
				return LoadShader (entryPath.filename ().string (), shaderType, stream, entry.size, assets);
			})) {
				Log (LogLevel::Error, "Cannot load shader: %s from %s pak!", entryPath.filename ().string ().c_str (), name.c_str ());
				return nullptr;
			}
		} else if (ext == ".dae") {
			if (!pak->ReadFile (entry, [&entryPath, &entry, &scenes] (std::istream& stream) -> bool {
				std::shared_ptr<aiScene> scene = LoadCollada (stream, entry.size);
				if (scene == nullptr) {
					return false;
				}

				std::string sceneName = entryPath.filename ().string ();

				std::shared_ptr<ColladaSceneInfo> sceneInfo;
				auto it = scenes.find (sceneName);
				if (it == scenes.end ()) {
					sceneInfo = std::make_shared<ColladaSceneInfo> ();
					scenes.emplace (sceneName, sceneInfo);
				} else {
					sceneInfo = it->second;
				}

				if (sceneInfo == nullptr) {
					return false;
				}

				sceneInfo->name = sceneName;
				sceneInfo->scene = scene;
				return true;
			})) {
				Log (LogLevel::Error, "Cannot load collada: %s from %s pak!", entryPath.filename ().string ().c_str (), name.c_str ());
				return nullptr;
			}
		} else if (ext == ".png") {
			if (!pak->ReadFile (entry, [&entryPath, &entry, &scenes](std::istream& stream) -> bool {
				std::string texName = entryPath.filename ().string ();
				std::shared_ptr<Texture> tex = LoadTexture (texName, stream, entry.size);
				if (tex == nullptr) {
					return false;
				}

				std::string sceneName = entryPath.parent_path ().string ();

				std::shared_ptr<ColladaSceneInfo> sceneInfo;
				auto it = scenes.find (sceneName);
				if (it == scenes.end ()) {
					sceneInfo = std::make_shared<ColladaSceneInfo> ();
					scenes.emplace (sceneName, sceneInfo);
				} else {
					sceneInfo = it->second;
				}

				if (sceneInfo == nullptr) {
					return false;
				}

				sceneInfo->name = sceneName;
				sceneInfo->textures.emplace (texName, tex);
				return true;
			})) {
				Log (LogLevel::Error, "Cannot load png texture: %s from %s pak!", entryPath.filename ().string ().c_str (), name.c_str ());
				return nullptr;
			}
		}
	}

	//Compile shader programs
	for (const std::string& program : programs) {
		std::vector<uint32_t> shaderIDs;

		auto itVertex = programVertexShaders.find (program);
		if (itVertex != programVertexShaders.end ()) {
			std::transform (itVertex->second.begin (), itVertex->second.end (), std::back_inserter (shaderIDs), [assets] (const std::string& shaderName) -> uint32_t {
				auto it = assets->vertexShaders.find (shaderName);
				if (it == assets->vertexShaders.end ()) {
					return 0;
				}
				return it->second;
			});
		}

		auto itFragment = programFragmentShaders.find (program);
		if (itFragment != programFragmentShaders.end ()) {
			std::transform (itFragment->second.begin (), itFragment->second.end (), std::back_inserter (shaderIDs), [assets] (const std::string& shaderName) -> uint32_t {
				auto it = assets->fragmentShaders.find (shaderName);
				if (it == assets->fragmentShaders.end ()) {
					return 0;
				}
				return it->second;
			});
		}

		auto linkRes = LinkProgram (program, shaderIDs, shaderBindCallback);
		if (!std::get<0> (linkRes)) {
			Log (LogLevel::Error, "Cannot link program: %s in %s pak!", program.c_str (), name.c_str ());
		}

		assets->programs.emplace (program, std::get<1> (linkRes));
	}

	//Create material infos and meshes
	for (auto& it : scenes) {
		std::shared_ptr<ColladaSceneInfo> info = it.second;

		std::map<std::string, uint32_t> materialShaderIDs;
		auto itMaterialShader = sceneMaterialShaders.find (info->name);
		if (itMaterialShader != sceneMaterialShaders.end ()) {
			const std::map<std::string, std::string>& materialProgramNames = itMaterialShader->second;
			for (auto& itMaterialProgramName : materialProgramNames) {
				auto itProgram = assets->programs.find (itMaterialProgramName.second);
				if (itProgram != assets->programs.end ()) {
					uint32_t programID = itProgram->second;
					materialShaderIDs.emplace (itMaterialProgramName.first, programID);
				}
			}
		}


		std::vector<std::shared_ptr<Material>> materials;
		for (uint32_t i = 0; i < info->scene->mNumMaterials; ++i) {
			materials.push_back (std::make_shared<Material> (info->scene->mMaterials[i], materialShaderIDs, info->textures));
		}

		assets->meshes.emplace (info->name, std::make_shared<Mesh> (info->name, info->scene->mMeshes[0], materials));
	}

	return assets;
}

void Scene::ReleaseAssets (std::shared_ptr<Assets>& assets) {
	for (auto it : assets->programs) {
		gl::DeleteProgram (it.second);
	}
	assets->programs.clear ();

	for (auto it : assets->vertexShaders) {
		gl::DeleteShader (it.second);
	}
	assets->vertexShaders.clear ();

	for (auto it : assets->fragmentShaders) {
		gl::DeleteShader (it.second);
	}
	assets->fragmentShaders.clear ();

	assets = nullptr;
}
