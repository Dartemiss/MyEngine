#pragma once

#include<list>
#include "Globals.h"
#include "Module.h"

class ModuleRender;
class ModuleWindow;
class ModuleTextures;
class ModuleInput;
class ModuleRenderExercise;
class ModuleProgram;
class ModuleIMGUI;
class ModuleTexture;
class ModuleCamera;
class ModuleModelLoader;
class ModuleTimeManager;
class ModuleScene;
class ModuleDebugDraw;
class ModuleFilesystem;

class Application
{
public:

	Application();
	~Application();

	bool Init();
	update_status Update();
	bool CleanUp();

public:
	ModuleRender* renderer = nullptr;
	ModuleWindow* window = nullptr;
	ModuleInput* input = nullptr;
	ModuleProgram* program = nullptr;
	ModuleIMGUI* imgui = nullptr;
	ModuleTexture* texture = nullptr;
	ModuleCamera* camera = nullptr;
	ModuleModelLoader* modelLoader = nullptr;
	ModuleTimeManager* timemanager = nullptr;
	ModuleScene* scene = nullptr;
	ModuleDebugDraw* debugDraw = nullptr;
	ModuleFilesystem* filesystem = nullptr;


private:

	std::list<Module*> modules;

};

extern Application* App;
