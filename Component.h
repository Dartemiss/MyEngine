#ifndef __Component_H__
#define __Component_H__

#include "Globals.h"
#include <string>

enum ComponentType
{
	TRANSFORM = 0,
	MESH,
	MATERIAL,
	CAMERA
};

class GameObject;
class SceneLoader;


class Component
{
public:

	Component()
	{
	}

	Component(const char* componentName)
	{
		name = componentName;
	}


	virtual void Enable()
	{
		return;
	}

	virtual void Disable()
	{
		return;
	}

	virtual void Update()
	{

		return;
	}

	virtual bool CleanUp()
	{
		return true;
	}

	virtual void OnSave(SceneLoader & loader) {}

	virtual void OnLoad(SceneLoader & loader) {}

	virtual void DrawInspector() 
	{
		return;
	}

	//GameObject Parent: 
	//IMPORTANT, THIS POINTER MUST BE SYNC WITH PARENTS CHILDREN, BE CAREFUL
	GameObject* myGameObject = nullptr;
	ComponentType myType = TRANSFORM;
	bool isActive = true;
	std::string name = "NewComponent";
};

#endif __Component_H__