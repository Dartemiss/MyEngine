#ifndef __GameObject_H__
#define __GameObject_H__

#include "Globals.h"
#include "Component.h"
#include "Dependencies/MathGeoLib/include/Geometry/AABB.h"
#include <string>
#include <vector>


//Class declaration is way faster than include header
class ComponentTransform;
class ComponentMesh;
class ComponentMaterial;
class ComponentCamera;
class SceneLoader;

class GameObject
{
public:
	GameObject();
	GameObject(const char* name);
	GameObject(const GameObject &go, GameObject* parent);
	~GameObject();

	//Core
	void Update();
	void SetParent(GameObject* newParent);
	void RemoveChildren(GameObject* child);
	void DeleteGameObject();
	void CleanUp();

	//Component Creation
	Component* CreateComponent(ComponentType type);

	//Hierarchy
	void DrawHierarchy(GameObject* selected);

	//Draw Main Camera
	void DrawCamera();

	//Update
	void UpdateTransform();

	//Variables
	//UID are unique
	unsigned int UID;

	//All GameObjects have a transform
	ComponentTransform* myTransform = nullptr;
	ComponentMesh* myMesh = nullptr;
	ComponentMaterial* myMaterial = nullptr;

	//Parent
	GameObject* parent = nullptr;

	//Chilren
	std::vector<GameObject*> children;

	//Components assigned to gameObject
	std::vector<Component*> components;
	
	//Name
	void SetName(const std::string &newName);
	std::string GetName() const;

	//UID substitute
	bool isRoot = false;
	bool isEnabled = true;
	bool isStatic = false;
	bool isParentOfMeshes = false;

	//Compute
	void ComputeAABB();
	void DrawAABB() const;

	AABB* boundingBox = nullptr;
	AABB* globalBoundingBox = nullptr;

	void DrawInspector(bool &showInspector);

	//Shape type
	enum ShapeType shape;

	void OnSave(SceneLoader & loader);
	void OnLoad(SceneLoader & loader);

	int numberOfCopies = 0;

	float IsIntersectedByRay(const float3 &origin, const LineSegment & ray);


private:
	std::string name = "";

	void CheckDragAndDrop(GameObject* go);


};

#endif __GameObject_H__