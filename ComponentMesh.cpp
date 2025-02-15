#include "Application.h"
#include "ModuleModelLoader.h"
#include "ComponentTransform.h"
#include "GameObject.h"
#include "ComponentMesh.h"
#include "SceneLoader.h"
#include "SceneImporter.h"
#include "MeshImporter.h"
#include "MathGeoLib/Geometry/LineSegment.h"
#include "MathGeoLib/Geometry/Triangle.h"

using namespace std;

ComponentMesh::ComponentMesh(GameObject* go)
{
	myGameObject = go;
	myType = MESH;
}

ComponentMesh::ComponentMesh(GameObject * go, ComponentMesh * comp)
{
	myGameObject = go;
	myType = MESH;
	mesh = comp->mesh;
}


ComponentMesh::~ComponentMesh()
{
	delete mesh;
}

void ComponentMesh::LoadMesh(Mesh* loadedMesh)
{
	mesh = loadedMesh;
}

void ComponentMesh::Draw(const unsigned int program) const
{
	mesh->Draw(program);
}

float ComponentMesh::IsIntersectedByRay(const float3 &origin ,const LineSegment & ray)
{
	float minDist = -1.0f;

	for(unsigned int i = 0; i < mesh->indices.size();i += 3)
	{

		Triangle tri = Triangle(mesh->vertices[mesh->indices[i]].Position, mesh->vertices[mesh->indices[i+1]].Position, mesh->vertices[mesh->indices[i+2]].Position);
		if(tri.Intersects(ray))
		{
			//If first time assign otherwise only assign if lesser
			float dist = tri.Distance(origin);
			if (minDist == -1.0f)
				minDist = dist;
			else if (dist < minDist)
				minDist = dist;
		}
	}


	return minDist;
}

void ComponentMesh::OnSave(SceneLoader & loader)
{
	loader.AddUnsignedInt("Type", myType);
	
	loader.AddString("meshName", mesh->name.c_str());
}

void ComponentMesh::OnLoad(SceneLoader & loader)
{
	string meshName = loader.GetString("meshName", "error");
	if (meshName != "error")
	{
		MeshData data;
		Importer->LoadMesh(meshName.c_str(), data);
		mesh = new Mesh();
		ProcessMeshData(data, *mesh);
		mesh->setupMesh();
	}
}


void ComponentMesh::ProcessMeshData(const MeshData & data, Mesh & myMesh)
{
	for (unsigned int i = 0; i < data.num_vertices; i++)
	{
		Vertex vertex;
		float3 positions;
		positions.x = data.positions[i * 3];
		positions.y = data.positions[i * 3 + 1];
		positions.z = data.positions[i * 3 + 2];
		vertex.Position = positions;

		float3 normals;
		normals.x = data.normals[i * 3];
		normals.y = data.normals[i * 3 + 1];
		normals.z = data.normals[i * 3 + 2];
		vertex.Normal = normals;

		//TODO: check if mesh contains texture coords or not
		float2 texturesCoords;
		texturesCoords.x = data.texture_coords[i * 2];
		texturesCoords.y = data.texture_coords[i * 2 + 1];
		vertex.TexCoords = texturesCoords;

		myMesh.vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < data.num_indices; i++)
	{
		myMesh.indices.push_back(data.indices[i]);
	}

	myMesh.name = data.name;
}

void ComponentMesh::DrawInspector()
{
	if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
	{
		//TODO: Make that isActive is used
		ImGui::Checkbox("Active", &isActive);
		ImGui::SameLine();
		if (ImGui::Button("Remove Component", ImVec2(130, 20)))
		{
			LOG("Removing Component Mesh from %s", myGameObject->name);
			myGameObject->components.erase(std::find(myGameObject->components.begin(), myGameObject->components.end(), this));
			CleanUp();
			delete this;

			return;
		}

		ImGui::Text("Path: %s", mesh->name.c_str());
		ImGui::Text("Number of triangles: %d", mesh->indices.size() / 3);

	}

	return;
}

