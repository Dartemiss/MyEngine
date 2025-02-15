#ifndef __ModuleTimeManager_H__
#define __ModuleTimeManager_H__

#include "Globals.h"
#include "Module.h"
#include "Timer.h"

class Application;

class ModuleTimeManager : public Module
{
public:
	ModuleTimeManager();
	~ModuleTimeManager();
	
	//Core
	bool Init();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

	//Methods
	void InitDeltaTimes();
	void FinalDeltaTimes();
	float GetGameTime() const;
	float GetRealGameTime() const;
	float GetDeltaTime() const;
	float GetRealDeltaTime() const;
	float GetTimeBeforeVsync() const;
	void ComputeTimeBeforeVsync();
	void PauseGame();
	void ExecuteNextFrames(int numberFrames);
	void Wait(float timeToWait);
	void PlayGame();

	//Variables
	long long frameCount = 0;
	float timeBeforeVsync = 0.0f;
	int FPS = 60;
	int fixedFPS = 60;
	float timeScale = 1.0f;
	bool isPaused = false;
	bool waitingToPause = false;
	bool fixFPS = false;
	bool isPlaying = false;

private:

	Timer* realTimer;
	
	float gameTime = 0.0f;
	float deltaTime = 0.0f;
	float realGameTime = 0.0f;
	float realDeltaTime = 0.0f;

	float initialGameFrameTime = 0.0f;
	float initialRealFrameTime = 0.0f;

	float timePaused = 0.0f;

	long long framesToPause = 0;

	unsigned int counterFPS = 0;
	float counterTimeFPS = 0.0f;
};

#endif __ModuleTimeManager_H__