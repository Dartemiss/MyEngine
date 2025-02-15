#ifndef _MODULE_DEBUGDRAW_H_
#define _MODULE_DEBUGDRAW_H_

#include "Module.h"

class DDRenderInterfaceCoreGL;
class ModuleCamera;

class ModuleDebugDraw : public Module
{

public:

	ModuleDebugDraw();
	~ModuleDebugDraw();

	bool            Init();
	update_status   Update();
	bool            CleanUp();

	void            Draw(ModuleCamera* camera, unsigned fbo, unsigned fb_width, unsigned fb_height);
private:

	static DDRenderInterfaceCoreGL* implementation;
};

#endif /* _MODULE_DEBUGDRAW_H_ */
