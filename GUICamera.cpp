#include "GUICamera.h"
#include "Application.h"
#include "ModuleCamera.h"
#include "ModuleRender.h"
#include "ModuleScene.h"
#include "Skybox.h"


void GUICamera::Draw(const char * title)
{
	if (isEnabled) 
	{

		ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
		ImGui::Begin(title, &isEnabled);

		ImGui::SliderFloat("FOV", &App->camera->editorCamera->frustum->verticalFov, 0.100f, 3.000f, "%.3f");
		App->camera->SetFOV();

		ImGui::Text("Camera Position: (%.3f,%.3f,%.3f)", App->camera->editorCamera->frustum->pos.x, App->camera->editorCamera->frustum->pos.y, App->camera->editorCamera->frustum->pos.z);
		ImGui::Text("Camera near distance: %.3f", App->camera->editorCamera->frustum->nearPlaneDistance);
		ImGui::Text("Camera far distance: %.3f", App->camera->editorCamera->frustum->farPlaneDistance);
		ImGui::Text("Time for building iterative quadtree: %f", App->scene->timeIterative);

		ImGui::Checkbox("Show Grid", &App->renderer->showGrid);
		ImGui::Checkbox("Show Bounding Box", &App->renderer->showBoundingBox);
		//Load Skybox only if is activated
		if(ImGui::Checkbox("Show Skybox", &App->renderer->showSkybox))
		{
			if(App->renderer->showSkybox)
			{
				App->renderer->skybox = new Skybox();
			}
			else
			{
				if(App->renderer->skybox != nullptr)
				{
					delete App->renderer->skybox;
					App->renderer->skybox = nullptr;
				}
			}

		}
		ImGui::Checkbox("Show QuadTree", &App->renderer->showQuadTree); 
		ImGui::Checkbox("Show AABBTree", &App->renderer->showAABBTree);
		ImGui::Checkbox("Show Frustum", &App->renderer->showFrustum);
		//ImGui::Checkbox("Move Objects", &App->scene->moveObjectsArround);

		/*
		if(ImGui::Button("Generate Cubes"))
		{
			App->scene->CreateCubesScript();
		}
		if (ImGui::Button("Generate Shapes"))
		{
			App->scene->CreateShapesScript();
		}
		
		if (ImGui::Button("Generate 1000 Houses"))
		{
			App->scene->CreateHousesScript();
		}
		*/
		ImGui::End();

	}


}
