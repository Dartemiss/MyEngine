#ifndef __MaterialImporter_H__
#define __MaterialImporter_H__

#include "MyImporter.h"

struct Texture;

class MaterialImporter : public MyImporter
{
public:
	MaterialImporter();

	bool Import(const char* path, const char* file, std::string& output_file);
	bool Import(const char* file, const void* buffer, unsigned int size, std::string& output_file);
	bool Load(const char* exported_file, Texture& resource);
	bool LoadSkyBox(const char* exported_file, Texture& resource, unsigned int &im);
	//bool LoadCheckers(Texture* resource);
};

#endif __MaterialImporter_H__