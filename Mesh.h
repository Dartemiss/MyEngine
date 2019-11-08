#ifndef __Mesh_H__
#define __Mesh_H__

#include "Globals.h"
#include "Module.h"
#include "include/Math/float3.h"
#include <string>
#include <vector>
#include "include/Math/float2.h"



struct Vertex {
	float3 Position;
	float3 Normal;
	float2 TexCoords;
};

struct Texture {
	unsigned int id;
	std::string type;
};

class Mesh
{
public:
	/*  Mesh Data  */
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	/*  Functions  */
	void Init();
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	~Mesh();
	void Draw(unsigned int shader);
private:
	/*  Render data  */
	unsigned int VAO, VBO, EBO;
	/*  Functions    */
	void setupMesh();
};
#endif __Mesh_H__
