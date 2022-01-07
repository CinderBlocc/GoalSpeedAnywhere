#include "GoalSpeedAnywhere.h"
#include "bakkesmod\wrappers\includes.h"
#include <sstream>
#include <iomanip>
#include <unordered_set>

BAKKESMOD_PLUGIN(GoalSpeedAnywhere, "Show the goal speed in any game mode", "1.1", PLUGINTYPE_FREEPLAY)


void GoalSpeedAnywhere::onLoad()
{
	bEnabled = std::make_shared<bool>(false);
	bDropShadow = std::make_shared<bool>(false);
	Duration = std::make_shared<float>(0.f);
	XPos = std::make_shared<int>(0);
	YPos = std::make_shared<int>(0);
	DecimalPrecision = std::make_shared<int>(0);
    TextColor = std::make_shared<LinearColor>();
	cvarManager->registerCvar("GSA_Enable", "1", "Show goal speed anywhere", true, true, 0, true, 1).bindTo(bEnabled);
	cvarManager->registerCvar("GSA_Shadow", "1", "Goal speed anywhere text drop shadow", true, true, 0, true, 1).bindTo(bDropShadow);
	cvarManager->registerCvar("GSA_Duration", "2", "Goal speed anywhere display duration", true, true, 0, true, 10).bindTo(Duration);
	cvarManager->registerCvar("GSA_X_Position", "25", "Goal speed anywhere X position", true, true, 0, true, 1920).bindTo(XPos);
	cvarManager->registerCvar("GSA_Y_Position", "245", "Goal speed anywhere Y position", true, true, 0, true, 1080).bindTo(YPos);
	cvarManager->registerCvar("GSA_Decimal_Precision", "2", "Goal speed anywhere decimal places to display", true, true, 0, true, 6).bindTo(DecimalPrecision);
	cvarManager->registerCvar("GSA_Color", "(0, 255, 0, 255)", "Goal speed anywhere text color", true).bindTo(TextColor);

	gameWrapper->HookEvent("Function TAGame.Ball_TA.OnHitGoal", std::bind(&GoalSpeedAnywhere::ShowSpeed, this));
	gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", std::bind(&GoalSpeedAnywhere::ShowSpeed, this));
	gameWrapper->HookEvent("Function Engine.GameViewportClient.Tick", std::bind(&GoalSpeedAnywhere::GetSpeed, this));
	
	gameWrapper->RegisterDrawable(bind(&GoalSpeedAnywhere::Render, this, std::placeholders::_1));
}
void GoalSpeedAnywhere::onUnload() {}

void GoalSpeedAnywhere::ShowSpeed()
{
	if(!(*bEnabled)) return;
	bShowSpeed = true;

	gameWrapper->SetTimeout(std::bind(&GoalSpeedAnywhere::HideSpeed, this), *Duration);
}

void GoalSpeedAnywhere::HideSpeed()
{
	bShowSpeed = false;
}

void GoalSpeedAnywhere::GetSpeed()
{
	if(!(*bEnabled) || bShowSpeed) return;
	ServerWrapper server = GetCurrentGameState();
	if(server.IsNull()) return;
	GameSettingPlaylistWrapper playlist = server.GetPlaylist();
	if(playlist.IsNull()) return;

	// Exclude some game modes where goals are not always outside of arena bounds, or horizontal
	static const std::unordered_set<int> excludedPlaylistIds = { 15, 17, 18, 19, 23 }; // Snow Day, Hoops, Rumble, Workshop, Dropshot
	if(excludedPlaylistIds.count(playlist.GetPlaylistId()) > 0) return;

	BallWrapper ball = server.GetBall();
	if(ball.IsNull()) return;

	Speed = ball.GetVelocity().magnitude();

	auto ballRadius = ball.GetRadius();

	ArrayWrapper<GoalWrapper> goalWrappers = server.GetGoals();
	for(auto goalWrapper : goalWrappers)
	{
		auto location = goalWrapper.GetLocation();
		if(!ballIsInsideGoal && abs(ball.GetLocation().Y) >= (abs(location.Y) + ballRadius) )
		{
			ballIsInsideGoal = true;
			ShowSpeed();
		}
		else if(ballIsInsideGoal && abs(ball.GetLocation().Y) < (abs(location.Y) + ballRadius))
		{
			ballIsInsideGoal = false;
		}
	}
}

void GoalSpeedAnywhere::Render(CanvasWrapper canvas)
{
	if(!bShowSpeed || !(*bEnabled)) return;
	
    canvas.SetColor(*TextColor);
	canvas.SetPosition(Vector2{*XPos,*YPos});
	
    if(gameWrapper->GetbMetric())
		canvas.DrawString("Goal Speed: " + ToStringPrecision(Speed * .036f, *DecimalPrecision) + " KPH", 3, 3, *bDropShadow);
	else
		canvas.DrawString("Goal Speed: " + ToStringPrecision(Speed / 44.704f, *DecimalPrecision) + " MPH", 3, 3, *bDropShadow);
}

ServerWrapper GoalSpeedAnywhere::GetCurrentGameState()
{
	if(gameWrapper->IsInOnlineGame())
		return gameWrapper->GetOnlineGame();
	else if(gameWrapper->IsInReplay())
		return gameWrapper->GetGameEventAsReplay().memory_address;
	else
		return gameWrapper->GetGameEventAsServer();
}

std::string GoalSpeedAnywhere::ToStringPrecision(float InValue, int Precision)
{
	std::ostringstream out;
	out << std::fixed << std::setprecision(Precision) << InValue;
	return out.str();
}
