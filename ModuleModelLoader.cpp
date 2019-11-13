#include "ModuleModelLoader.h"
#include "Application.h"
#include"ModuleTexture.h"
#include "ModuleCamera.h"
#include <cimport.h>
#include <postprocess.h>
#include <material.h>
#include <mesh.h>
#include "Timer.h"
#include "uSTimer.h"

ModuleModelLoader::ModuleModelLoader()
{
}


ModuleModelLoader::~ModuleModelLoader()
{
}

bool ModuleModelLoader::Init()
{

	uSTimer initTimer;
	initTimer.StartTimer();

	loadModel("../Models/baker_house/BakerHouse.fbx");
	//loadModel("C:\\Users\\Riqui\\Documents\\GitHub\\MyEngine\\Models\\baker_house\\BakerHouse.fbx");

	//loadModel("../Models/spongebob/spongebob.fbx");

	float time = initTimer.StopTimer();
	LOG("Loader takes %.5f miliseconds", time);



	//loadModel("../Models/dragon/blackdragon.fbx");
	//loadModel("../Models/axe/machado.fbx");
	//loadModel("../Models/nanosuit/scene.fbx");
	//loadModel("../Models/penguin/PenguinBaseMesh.fbx");

	
	
	return true;
}

update_status ModuleModelLoader::PreUpdate()
{
	return UPDATE_CONTINUE;
}

update_status ModuleModelLoader::Update()
{
	return UPDATE_CONTINUE;
}

update_status ModuleModelLoader::PostUpdate()
{
	return UPDATE_CONTINUE;
}

bool ModuleModelLoader::CleanUp()
{
	emptyScene();
	
	return true;
}

void ModuleModelLoader::Draw(unsigned int program)
{
	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i]->Draw(program);
}


void ModuleModelLoader::loadModel(const std::string path)
{
	if (isModelLoaded)
		emptyScene();

	LOG("Importing model \n");
	const aiScene* scene = aiImportFile(path.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG("ERROR ASSIMP: %s \n", aiGetErrorString());
		return;
	}

	directory = computeDirectory(path);

	if (directory == "")
		return;
	processNode(scene->mRootNode, scene);
	computeModelBoundingBox();
	isModelLoaded = true;

}

void ModuleModelLoader::processNode(aiNode * node, const aiScene * scene)
{
	LOG("For each mesh located on the current node, processing meshes.")
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(new Mesh(processMesh(mesh, scene)));
	}
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

Mesh ModuleModelLoader::processMesh(aiMesh * mesh, const aiScene * scene)
{
	//Filling data
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		// process vertex positions, normals and texture coordinates
		float3 positions;
		positions.x = mesh->mVertices[i].x;
		positions.y = mesh->mVertices[i].y;
		positions.z = mesh->mVertices[i].z;
		vertex.Position = positions;

		float3 normals;
		normals.x = mesh->mNormals[i].x;
		normals.y = mesh->mNormals[i].y;
		normals.z = mesh->mNormals[i].z;
		vertex.Normal = normals;

		
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			float2 texturesCoords;
			texturesCoords.x = mesh->mTextureCoords[0][i].x;
			texturesCoords.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = texturesCoords;
		}
		else
			vertex.TexCoords = float2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}
	// process indices of each face
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
		// process material
		
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
			// 1. diffuse maps
			std::vector<Texture> diffuseMaps = App->texture->loadMaterialTextures(material,
				aiTextureType_DIFFUSE, "texture_diffuse", directory);
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			// 2. specular maps
			std::vector<Texture> specularMaps = App->texture->loadMaterialTextures(material,
				aiTextureType_SPECULAR, "texture_specular", directory);
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			// 3. normal maps
			std::vector<Texture> normalMaps = App->texture->loadMaterialTextures(material,
				aiTextureType_HEIGHT, "texture_normal",directory);
			textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
			// 4. height maps
			std::vector<Texture> heightMaps = App->texture->loadMaterialTextures(material,
				aiTextureType_AMBIENT, "texture_height",directory);
			textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
		}
		
	return Mesh(vertices, indices, textures);
}

std::string ModuleModelLoader::computeDirectory(const std::string path)
{
	size_t simpleRightSlash = path.find_last_of('/');
	if (std::string::npos != simpleRightSlash)
	{
		LOG("Directory with simpleRightSlashes.")
		return path.substr(0, path.find_last_of('/') + 1);
	}
	size_t doubleRightSlash = path.find_last_of('//');
	if (std::string::npos != doubleRightSlash)
	{
		LOG("Directory with doubleRightSlashes.")
		return path.substr(0, path.find_last_of('//') + 1);
	}

	size_t doubleLeftSlash = path.find_last_of('\\');
	if (std::string::npos != doubleLeftSlash)
	{
		LOG("Directory with doubleLeftSlashes.")
		return path.substr(0, path.find_last_of('\\') + 1);
	}

	LOG("ERROR: Invalid path.");
	return "";
}

void ModuleModelLoader::emptyScene()
{
	for(auto mesh : meshes)
	{
		delete mesh;
	}

	meshes.clear();
	modelBox.clear();
}

void ModuleModelLoader::computeModelBoundingBox()
{
	LOG("Computing Bounding Box from min and max values of the vertices of each mesh.");

	//Min values
	float minX = 10000000.0f;
	float minY = 10000000.0f;
	float minZ = 10000000.0f;

	//Max values
	float maxX = -1000000.0f;
	float maxY = -1000000.0f;
	float maxZ = -1000000.0f;

	for(auto mesh: meshes)
	{
		for(auto vertex : mesh->vertices)
		{
			//Min vertex
			if (vertex.Position.x < minX)
				minX = vertex.Position.x;

			if (vertex.Position.y < minY)
				minY = vertex.Position.y;

			if (vertex.Position.z < minZ)
				minZ = vertex.Position.z;

			//Max vertex
			if (vertex.Position.x > maxX)
				maxX = vertex.Position.x;

			if (vertex.Position.y > maxY)
				maxY = vertex.Position.y;

			if (vertex.Position.z > maxZ)
				maxZ = vertex.Position.z;
		}
	}
	//Representation of a Cube, have exactly 8 vertex
	//Order of representation:
	//0-> (-x,-y,-z), 1-> (x,-y,-z), 2-> (x,-y,z), 3-> (-x,-y,z)
	//4-> (-x,y,-z), 5-> (x,y,-z), 6-> (x,y,z), 7-> (-x,y,z)

	modelBox.push_back(float3(minX, minY, minZ));
	modelBox.push_back(float3(maxX, minY, minZ));
	modelBox.push_back(float3(maxX,minY,maxZ));
	modelBox.push_back(float3(minX, minY, maxZ));
	modelBox.push_back(float3(minX, maxY, minZ));
	modelBox.push_back(float3(maxX, maxY, minZ));
	modelBox.push_back(float3(maxX, maxY, maxZ));
	modelBox.push_back(float3(minX, maxY, maxZ));


	correctCameraPositionForModel = float3((modelBox[1].x + modelBox[0].x)/2, (modelBox[4].y + modelBox[0].y)/2, 2*modelBox[3].z);
	App->camera->TranslateCameraToPoint(correctCameraPositionForModel);
}

