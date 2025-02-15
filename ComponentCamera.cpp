#include "ComponentCamera.h"
#include "ComponentTransform.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleProgram.h"
#include "ModuleCamera.h"
#include "ModuleRender.h"
#include "GameObject.h"
#include "SceneLoader.h"
#include <math.h>
#include "MathGeoLib/Geometry/Plane.h"
#include "debugdraw.h"
#include "GL/glew.h"


ComponentCamera::ComponentCamera(GameObject* go)
{
	myGameObject = go;
	myType = CAMERA;

	frustum = new Frustum();
	frustum->type = FrustumType::PerspectiveFrustum;
	frustum->pos = float3::zero;
	frustum->front = -float3::unitZ;
	frustum->up = float3::unitY;
	frustum->nearPlaneDistance = 0.1f;
	frustum->farPlaneDistance = 50.0f;
	frustum->verticalFov = (float)M_PI / 4.0f;
	aspect = 1.0f;
	frustum->horizontalFov = 2.0f * atanf(tanf(frustum->verticalFov * 0.5f) *aspect);

	frustum->Translate(float3(0.0f, 0.0f, 0.0f));

	proj = frustum->ProjectionMatrix();
	view = frustum->ViewMatrix();

}

ComponentCamera::ComponentCamera(GameObject * go, ComponentCamera * comp)
{
	myGameObject = go;
	myType = CAMERA;

	frustum = new Frustum(*comp->frustum);

	proj = frustum->ProjectionMatrix();
	view = frustum->ViewMatrix();

}


ComponentCamera::~ComponentCamera()
{
	delete frustum;
}

void ComponentCamera::Update()
{
	float3x3 quatAux = float3x3::zero;
	float3 scaleAux = float3::zero;
	myGameObject->myTransform->globalModelMatrix.Decompose(frustum->pos,quatAux, scaleAux);
	frustum->up = float3::unitY * quatAux;
	frustum->front = -float3::unitZ * quatAux;
	
	view = frustum->ViewMatrix();

	return;
}

bool ComponentCamera::CleanUp()
{
	return true;
}

void ComponentCamera::SetFOV()
{
	frustum->horizontalFov = 2.0f * atanf(tanf(frustum->verticalFov * 0.5f) *aspect);
	proj = frustum->ProjectionMatrix();
}

void ComponentCamera::SetAspectRatio(int newWidth, int newHeight)
{
	aspect = ((float)newWidth / newHeight);
	frustum->horizontalFov = 2.0f * atanf(tanf(frustum->verticalFov * 0.5f) *aspect);

	proj = frustum->ProjectionMatrix();
}

void ComponentCamera::Rotate(const float dx, const float dy)
{
	if (dx != 0.0f)
	{
		float3x3 rotationY = float3x3::RotateY(dx);
		frustum->front = rotationY.Transform(frustum->front).Normalized();
		frustum->up = rotationY.Transform(frustum->up).Normalized();
	}

	if (dy != 0.0f)
	{
		float3x3 rotationX = float3x3::RotateAxisAngle(frustum->WorldRight(), dy);
		frustum->up = rotationX.Transform(frustum->up).Normalized();
		frustum->front = rotationX.Transform(frustum->front).Normalized();
	}
}

void ComponentCamera::Move(const float3 &direction)
{
	if (!direction.Equals(float3::zero))
		frustum->Translate(direction);
}


void ComponentCamera::TranslateCameraToPoint(const float3 & newPos)
{
	frustum->pos = newPos;
	frustum->front = -float3::unitZ;
	frustum->up = float3::unitY;


	view = frustum->ViewMatrix();
}

void ComponentCamera::SetNearPlaneDistance(const float nearDist)
{
	frustum->nearPlaneDistance = nearDist;
	proj = frustum->ProjectionMatrix();
}

void ComponentCamera::SetFarPlaneDistance(const float farDist)
{
	frustum->farPlaneDistance = farDist;
	proj = frustum->ProjectionMatrix();
}

void ComponentCamera::LookAt(const float3 &target)
{
	float3 dir = (target - frustum->pos).Normalized();
	float3x3 rot = float3x3::LookAt(frustum->front, dir, frustum->up, float3::unitY);
	frustum->front = rot.Transform(frustum->front).Normalized();
	frustum->up = rot.Transform(frustum->up).Normalized();
}

void ComponentCamera::ComputeViewMatrix() 
{
	view = frustum->ViewMatrix();
	return;
}

void ComponentCamera::ComputeProjMatrix()
{
	proj = frustum->ProjectionMatrix();
	return;
}

void ComponentCamera::DrawInspector()
{
	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{
		//TODO: Make that isActive is used
		ImGui::Checkbox("Active", &isActive);
		ImGui::SameLine();
		if(ImGui::Button("Remove Component",ImVec2(130,20)))
		{
			LOG("Removing Component Camera from %s", myGameObject->name);
			myGameObject->components.erase(std::find(myGameObject->components.begin(), myGameObject->components.end(), this));
			CleanUp();
			delete this;
			
			return;
		}

		ImGui::Combo(":Projection ", &currentPerspective, perspectives, IM_ARRAYSIZE(perspectives));
		ChangeFrustumType();

		float auxNearPlane = frustum->nearPlaneDistance;
		float auxFarPlane = frustum->farPlaneDistance;

		ImGui::SliderFloat(":NearPlane dist", &auxNearPlane, 0.0001f, 9.0f);
		ImGui::SliderFloat(":FarPlane dist", &auxFarPlane,10.0f, 1000.0f);

		SetNearPlaneDistance(auxNearPlane);
		SetFarPlaneDistance(auxFarPlane);

		ImGui::SliderFloat(":FOV", &frustum->verticalFov, 0.0001f, 9.0f);

		SetFOV();
	}

	return;
}

int ComponentCamera::AABBWithinFrustum(const AABB &aabb) const
{
	//Tests if an AABB is within the frusum
	//returns 0 if out, 1 if in and 2 if intersects
	float3 corners[8];
	aabb.GetCornerPoints(corners);

	int iTotalIn = 0;

	//test all 8 corners against the 6 planes of the frustum
	// if all points are behind 1 specific plane, we are out
	// if we are in with all points, then we are fully in
	for(int p = 0; p < 6; ++p)
	{
		int iInCount = 8;
		int iPtIn = 1;

		for(int i = 0; i< 8; ++i)
		{
			//test this point against the planes
			if(SideOfPlane(corners[i], frustum->GetPlane(p)) == FRONT)
			{
				iPtIn = 0;
				--iInCount;
			}
		}
		
		// were all the points outside of plane p?
		if (iInCount == 0)
			return AABB_OUT;

		// check if they were all on the right side of the plane
		iTotalIn += iPtIn;

	}
	// so if iTotalIn is 6, then all are inside the view
	if (iTotalIn == 6)
		return(AABB_IN);

	return AABB_INTERSECT;
}

bool ComponentCamera::SideOfPlane(const float3 &point, const Plane &plane) const
{
	float value = plane.normal.Dot(point);
	value -= plane.d;

	return (value >= 0.0f) ? true : false;
}

void ComponentCamera::OnSave(SceneLoader & loader)
{
	//TODO: fix camera loading when saved
	loader.AddUnsignedInt("Type", myType);
	loader.AddUnsignedInt("isMainCamera", isMainCamera);

	loader.AddUnsignedInt("Frustum Type", frustum->type);
	loader.AddVec3f("Position", frustum->pos);
	loader.AddVec3f("Front", frustum->front);
	loader.AddVec3f("Up", frustum->up);
	loader.AddFloat("Near Plane", frustum->nearPlaneDistance);
	loader.AddFloat("Far Plane", frustum->farPlaneDistance);
	loader.AddFloat("Vertical FOV", frustum->verticalFov);
	loader.AddFloat("Horizontal FOV", frustum->horizontalFov);
}

void ComponentCamera::OnLoad(SceneLoader & loader)
{
	frustum = new Frustum();
	isMainCamera = loader.GetUnsignedInt("isMainCamera", isMainCamera);

	frustum->type = (FrustumType)loader.GetUnsignedInt("Frustum Type", FrustumType::PerspectiveFrustum);
	frustum->pos = loader.GetVec3f("Position", float3::zero);
	frustum->front = loader.GetVec3f("Front", -float3::unitZ);
	frustum->up = loader.GetVec3f("Up", float3::unitY);
	frustum->nearPlaneDistance = loader.GetFloat("Near Plane", 0.1f);
	frustum->farPlaneDistance = loader.GetFloat("Far Plane", 50.0f);
	frustum->verticalFov = loader.GetFloat("Vertical FOV", (float)M_PI / 4.0f);
	frustum->horizontalFov = loader.GetFloat("Horizontal FOV", 2.0f * atanf(tanf(frustum->verticalFov * 0.5f)));

	proj = frustum->ProjectionMatrix();
	view = frustum->ViewMatrix();

	if(isMainCamera)
	{
		App->renderer->gameCamera = this;
	}
}

void ComponentCamera::DrawFrustum()
{
	//Draw frustum
	float4x4 clipMatrix = proj * view;	
	dd::frustum(clipMatrix.Inverted(), float3(0, 0, 1));

	return;
}

void ComponentCamera::ChangeFrustumType()
{
	if (perspectives[currentPerspective] != usedPerspective)
	{
		if (currentPerspective == 0)
		{
			frustum->type = FrustumType::PerspectiveFrustum;
		}
		else
		{
			frustum->type = FrustumType::OrthographicFrustum;
		}
		usedPerspective = perspectives[currentPerspective];
	}

	return;
}
