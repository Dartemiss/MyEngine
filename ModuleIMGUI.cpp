#include "ModuleIMGUI.h"
#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_sdl.h"
#include "Imgui/imgui_impl_opengl3.h"
#include "ImGuizmo/ImGuizmo.h"
#include "GL/glew.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleTexture.h"
#include "ModuleScene.h"
#include "ModuleRender.h"
#include "ModuleTimeManager.h"
#include <stdio.h>
#include <SDL/SDL.h>
#include "FontAwesome/IconsFontAwesome5.h"
#include "FontAwesome/IconsFontAwesome5Brands.h"


#define DOCUMENTATION "https://github.com/ocornut/imgui/wiki"
#define REPOSITORY "https://github.com/Dartemiss/DragonEngine"


ModuleIMGUI::ModuleIMGUI()
{
}


ModuleIMGUI::~ModuleIMGUI()
{
}

bool ModuleIMGUI::Init()
{
	ImGui::CreateContext();
	const char* glsl_version = "#version 330";
	ImGui_ImplSDL2_InitForOpenGL(App->window->window, App->window->glcontext);
	ImGui_ImplOpenGL3_Init(glsl_version);

	ImGui::StyleColorsDark();


	//Font Awesome Init
	ImGuiIO& io = ImGui::GetIO();

	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFontConfig icons_config; 
	icons_config.MergeMode = true; 
	icons_config.PixelSnapH = true;

	io.Fonts->AddFontDefault();
	io.Fonts->AddFontFromFileTTF("../Fonts/" FONT_ICON_FILE_NAME_FAS, 12.0f, &icons_config, icons_ranges);

	io.Fonts->AddFontDefault();
	io.Fonts->AddFontFromFileTTF("../Fonts/" FONT_ICON_FILE_NAME_FAR, 12.0f, &icons_config, icons_ranges);

	io.Fonts->AddFontDefault();
	static const ImWchar icons_ranges_fab[] = { ICON_MIN_FAB, ICON_MAX_FAB, 0 };
	io.Fonts->AddFontFromFileTTF("../Fonts/" FONT_ICON_FILE_NAME_FAB, 12.f, &icons_config, icons_ranges_fab);
	// use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid


	return true;
}

update_status ModuleIMGUI::PreUpdate()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(App->window->window);
	ImGui::NewFrame();
	return UPDATE_CONTINUE;
}

update_status ModuleIMGUI::Update()
{
	guiWindow.Draw("Window Configuration");
	console.Draw();
	about.Draw("About");
	guiCamera.Draw("Camera Settings");
	timeManager.Draw("Timers");
	inspector.Draw("Properties");

	//ImGui::ShowDemoWindow();

	ImGui::BeginTabBar("MainViewTabs");
	ImGui::EndTabBar();

	//Menu
	if (ImGui::BeginMainMenuBar()) 
	{

	
		if(ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("Repository"))
				ShellExecuteA(NULL, "open", REPOSITORY, NULL, NULL, SW_SHOWNORMAL);

			if (ImGui::MenuItem("Documentation"))
				ShellExecuteA(NULL, "open", DOCUMENTATION, NULL, NULL, SW_SHOWNORMAL);
			
			if(ImGui::MenuItem("About"))
			{
				about.ToggleEnable();
			}

			if(ImGui::MenuItem("Quit"))
			{
				return UPDATE_STOP;
			}

			ImGui::EndMenu();
		}

		if(ImGui::BeginMenu("Debug"))
		{
			if(ImGui::MenuItem("Console"))
			{
				console.ToggleEnable();
			}
			if (ImGui::MenuItem("Time"))
			{
				timeManager.ToggleEnable();
			}

			ImGui::EndMenu();
		}



		if(ImGui::BeginMenu("View"))
		{
			if(ImGui::MenuItem("Window"))
			{
				guiWindow.ToggleEnable();
			}

			if(ImGui::MenuItem("Camera"))
			{
				guiCamera.ToggleEnable();
			}

			if(ImGui::MenuItem("Show Hierarchy"))
			{
				App->scene->showHierarchy = true;
			}

			if (ImGui::MenuItem("Show Inspector"))
			{
				App->scene->showInspector = true;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Scene"))
		{
			if (ImGui::MenuItem("Save Scene"))
			{
				App->scene->SaveScene();
			}

			if (ImGui::MenuItem("Load Scene"))
			{
				App->scene->LoadScene();
			}

			ImGui::EndMenu();
		}
	}
	ImGui::EndMainMenuBar();

	return UPDATE_CONTINUE;
}

update_status ModuleIMGUI::PostUpdate()
{

	//Render
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	return UPDATE_CONTINUE;
}

bool ModuleIMGUI::CleanUp()
{
	console.Clear();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	

	return true;
}

void ModuleIMGUI::DrawPlayPauseButtons() const
{
	//Draw Scene and Game Windows	
	ImGui::SetCursorPos(ImVec2(App->window->width * 0.30f, App->window->height * 0.0055f));
	if(ImGui::Button(ICON_FA_PLAY ""))
	{
		//Do things
		App->timemanager->PlayGame();
		App->renderer->isGamePlaying = !App->renderer->isGamePlaying;
		//If now is going to be game
		if(App->renderer->isGamePlaying)
		{
			App->scene->SaveScene();
		}
		else
		{
			App->scene->LoadScene();
		}
	}
	ImGui::SameLine();
	if(ImGui::Button(ICON_FA_PAUSE ""))
	{
		//Do things
		App->timemanager->PauseGame();
	}
	ImGui::SameLine();
	if(ImGui::Button(ICON_FA_STEP_FORWARD ""))
	{
		//Do things
		App->timemanager->ExecuteNextFrames(2);
	}

	ImGui::SetCursorPos(ImVec2(App->window->width * 0.35f, App->window->height * 0.0055f));
	ImGui::Checkbox("MSAA", &App->renderer->antialiasing);

}
