#pragma once
#pragma comment(lib, "PluginSDK.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"

class GoalSpeedAnywhere : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	std::shared_ptr<bool> bEnabled;
	std::shared_ptr<bool> bDropShadow;
	std::shared_ptr<float> Duration;
	std::shared_ptr<int> XPos;
	std::shared_ptr<int> YPos;
    std::shared_ptr<LinearColor> TextColor;

	bool bShowSpeed = false;
	float Speed = 0;

public:
	void onLoad() override;
	void onUnload() override;

	void ShowSpeed();
	void HideSpeed();
	void GetSpeed();
	void Render(CanvasWrapper canvas);
	ServerWrapper GetCurrentGameState();
    std::string ToStringPrecision(float InValue, int Precision);
};
