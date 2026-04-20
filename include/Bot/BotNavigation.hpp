#pragma once

#include "Bot/SettingsManager.hpp"
#include "CharacterMovement.hpp"
#include "types.hpp"
#include "IO/WindowManager.hpp"
#include "IO/HIDController.hpp"

class BotNavigation
{
public:
    BotNavigation(HIDController& hidController,std::shared_ptr<WindowManager> wm);
    void updatePosition(std::pair<float,float> pos);
    void angleNavigation(float angle,WindowInfo windowInfo);
    bool run();
    void goToAttackingArea();
    void changeSpot();
    bool isInsideAttackingArea();
    bool isInsideCurrentSpot();
    void setState(NavigationState nState);
    NavigationState getState();
private:
    void processGoToBypassZone();
    void processChangeSpot();
    void processOutOfAttackingArea();

    BotSettings settings;
    NavigationState state;
    HIDController& hidController;
    std::shared_ptr<WindowManager> wm;

    std::pair<float,float> charPos;
    std::pair<float,float> charPosImg;
    std::pair<float,float> prevCharPos;
    std::pair<float,float> target;

    float correctionAngle=0;

    struct BotNavigationSettings
    {
        int movementRate=5;
        int movementRateCnt=0;
        int moveToPointErrThreshold=15;
        int trajectoryCnt = 0;
        int spotCnt = 0;
        bool bypassZoneReached=true;
    }botNavigationSettings;
};
