#include "stdafx.h"
#include "scene.hpp"
#include "pak.hpp"
#include "EglContext.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

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

std::tuple<bool, uint32_t> Scene::LinkProgram (const std::vector<uint32_t>& shaderIDs, std::function<void(uint32_t programID)> bindCallback) {
	// Create the shader program
	uint32_t programID = gl::CreateProgram ();

	// Attach the fragment and vertex shaders to it
	for (uint32_t shaderID : shaderIDs) {
		gl::AttachShader (programID, shaderID);
	}

	// Bind the custom vertex attribute "myVertex" to location VERTEX_ARRAY
	if (bindCallback) {
		bindCallback (programID);
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

bool Scene::LoadCollada (const std::string& colladaName, std::istream& stream, uint64_t len, std::shared_ptr<Assets> assets) {
	std::vector<uint8_t> data;
	data.resize (len);

	stream.read ((char*) &data[0], len);
	if (!stream) {
		return false;
	}

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFileFromMemory (&data[0], data.size (),
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (!scene) {
		return false;
	}

	assets->colladaScenes.emplace (colladaName, std::shared_ptr<aiScene> (importer.GetOrphanedScene ()));
	return true;
}

std::shared_ptr<Scene::Assets> Scene::LoadPak (const std::string& name, std::function<void (uint32_t programID)> shaderBindCallback) {
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
			if (!pak->ReadFile (entry, [&entryPath, &entry, assets] (std::istream& stream) -> bool {
				return LoadCollada (entryPath.filename ().string (), stream, entry.size, assets);
			})) {
				Log (LogLevel::Error, "Cannot load collada: %s from %s pak!", entryPath.filename ().string ().c_str (), name.c_str ());
				return nullptr;
			}
		} else if (ext == ".png") {
			//TODO: implement png texture loading...
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

		auto linkRes = LinkProgram (shaderIDs, shaderBindCallback);
		if (!std::get<0> (linkRes)) {
			Log (LogLevel::Error, "Cannot link program: %s in %s pak!", program.c_str (), name.c_str ());
		}

		assets->programs.emplace (program, std::get<1> (linkRes));
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
