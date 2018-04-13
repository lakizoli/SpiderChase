#pragma once

#define DECLARE_SCENE(cls)								\
	static std::shared_ptr<Scene> __create () {			\
		return std::make_shared<cls> ();				\
	}													\
	static uint32_t __register_result;

#define IMPLEMENT_SCENE(cls, sceneID)					\
	uint32_t cls::__register_result = Scene::RegisterSceneCreator (sceneID, cls::__create);

class Scene {
	bool _isInited;
	std::string _nextSceneID; ///< The ID of the scene have to execute after release of the current scene instance. When empty, then the game exits...

//Definitions
public:
	enum class SceneResults {
		Continue,
		EndScene
	};

//Construction
private:
	typedef std::shared_ptr<Scene> (*SceneCreator)();
	static std::map<std::string, SceneCreator>& GetCreatorMap ();

protected:
	Scene ();
	static uint32_t RegisterSceneCreator (const std::string& sceneID, SceneCreator sceneCreator);

public:
	static std::shared_ptr<Scene> Create (const std::string& sceneID);
	virtual ~Scene () = default;

//Game management interface
public:
	SceneResults Step ();

protected:
	virtual void Init () {};
	virtual void Release () {};

	virtual SceneResults Update (double frameTime) {
		return SceneResults::EndScene;
	}

	virtual void Render () {};

//Next scene handling
public:
	const std::string& GetNextSceneID () const {
		return _nextSceneID;
	}

protected:
	void SetNextSceneID (const std::string& sceneID) {
		_nextSceneID = sceneID;
	}

	void ClearNextSceneID () {
		_nextSceneID.clear ();
	}

//Asset pipeline functions
protected:
	struct Assets {
		std::map<std::string, uint32_t> fragmentShaders;
		std::map<std::string, uint32_t> vertexShaders;
		std::map<std::string, uint32_t> programs;
	};

private:
	static std::tuple<bool, uint32_t> CompileShader (uint32_t type, const char* source);
	static std::tuple<bool, uint32_t> LinkProgram (const std::vector<uint32_t>& shaderIDs, std::function<void (uint32_t programID)> bindCallback);
	static bool LoadShader (const std::string& shaderName, uint32_t shaderType, std::istream& stream, uint64_t len, std::shared_ptr<Assets> assets);
	static bool LoadProgram (const std::string& programName, std::istream& stream, uint64_t len, std::vector<std::string>& vertexShaders, std::vector<std::string>& fragmentShaders);

protected:
	static std::shared_ptr<Assets> LoadPak (const std::string& name, std::function<void (uint32_t programID)> shaderBindCallback);
	static void ReleaseAssets (std::shared_ptr<Assets>& assets);
};
