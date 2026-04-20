#include "Bot/BotNavigation.hpp"

BotNavigation::BotNavigation(HIDController& hidController,std::shared_ptr<WindowManager> wm):
    hidController{hidController},
    wm{wm},
    state{NavigationState::NONE},
    charPosImg{0.f, 0.f},
    charPos{0.f, 0.f},
    prevCharPos{0.f, 0.f}
{
    SettingsManager settingsManager;
    settings = settingsManager.loadSettings("assets/settings.json");

    movement::sortNavigationWrtDistanceFarestFirst(
        settings.activeNavigationStructure.trajectory,
        settings.activeNavigationStructure.attackingArea[0].pos);
}

void BotNavigation::updatePosition(std::pair<float,float> pos)
{
    charPos=pos;
    charPosImg = movement::convertToImgCoordinates(charPos);
}

void BotNavigation::angleNavigation(float angle,WindowInfo windowInfo)
{
    hidController.setMousePosition(
        frame_process::positionToWindow(
            windowInfo.windowPosition,
            movement::setMouseWrtChar(windowInfo.windowCenter,250,angle),
            windowInfo.screenResolution));

    if (!frame_process::checkIfMob(wm))
        hidController.leftClickMouse(
            frame_process::positionToWindow(
                windowInfo.windowPosition, 
                movement::setMouseWrtChar(windowInfo.windowCenter,250,angle),
                windowInfo.screenResolution));
}

bool BotNavigation::run()
{
    auto windowInfo = wm->getWindowInfo();
    
    if (botNavigationSettings.movementRateCnt>=botNavigationSettings.movementRate)
    {
        switch(state)
        {
            case NavigationState::OUT_OF_ATTACKING_AREA:
            {
                if(isInsideAttackingArea())
                {
                    state=NavigationState::GO_TO_BYPASS_ZONE;
                    std::cout<<"[LOG][NAVIGATION][" << utils::getTimestamp() << "] Attacking area reached, going to attacking spot.\n";
                    break;
                }

                float angleError = movement::getAngleError(charPos,prevCharPos,movement::convertToGameCoordinates(target));
                float distanceError = movement::getDistance(charPos,movement::convertToGameCoordinates(target));

                if (distanceError<botNavigationSettings.moveToPointErrThreshold)
                {
                    if (botNavigationSettings.trajectoryCnt < settings.activeNavigationStructure.trajectory.size())
                    {
                        target = settings.activeNavigationStructure.trajectory[++botNavigationSettings.trajectoryCnt].pos;
                    }
                }

                correctionAngle+=angleError*0.3;
                angleNavigation(correctionAngle,windowInfo);
            } break;

            case NavigationState::CHANGE_SPOT:
            {
                float angleError = movement::getAngleError(charPos,prevCharPos,movement::convertToGameCoordinates(target));
                float distanceError = movement::getDistance(charPos,movement::convertToGameCoordinates(target));

                if (distanceError<botNavigationSettings.moveToPointErrThreshold)
                {
                    botNavigationSettings.bypassZoneReached = false;
                    std::cout<<"[LOG][NAVIGATION][" << utils::getTimestamp() << "] Spot area reached.\n";

                    return true;
                }

                correctionAngle+=angleError*0.3;
                angleNavigation(correctionAngle,windowInfo);
            } break;
            case NavigationState::GO_TO_BYPASS_ZONE:
            {
                if (settings.activeNavigationStructure.bypass.size()>0 && botNavigationSettings.bypassZoneReached==false)
                {
                    float angleError = movement::getAngleError(charPos,prevCharPos,movement::convertToGameCoordinates(target));
                    float distanceError = movement::getDistance(charPos,movement::convertToGameCoordinates(target));

                    if (distanceError<botNavigationSettings.moveToPointErrThreshold)
                    {
                        if (botNavigationSettings.spotCnt < settings.activeNavigationStructure.toGoWhenNoMobs.size()-1)
                        {
                            botNavigationSettings.spotCnt++;
                        } else
                        {
                            botNavigationSettings.spotCnt = 0;                            
                        }
                        
                        target = settings.activeNavigationStructure.toGoWhenNoMobs[botNavigationSettings.spotCnt].pos;

                        botNavigationSettings.bypassZoneReached = true;
                        state = NavigationState::CHANGE_SPOT;
                        std::cout<<"[LOG][NAVIGATION][" << utils::getTimestamp() << "] Bypass zone reached, going to spot number "<<botNavigationSettings.spotCnt<<"\n";

                        return true;
                    }

                    correctionAngle+=angleError*0.3;
                    angleNavigation(correctionAngle,windowInfo);
                }
                else
                {
                    if (settings.activeNavigationStructure.bypass.size()==0)
                    {
                        if (botNavigationSettings.spotCnt < settings.activeNavigationStructure.toGoWhenNoMobs.size()-1)
                        {
                            botNavigationSettings.spotCnt++;
                        } else
                        {
                            botNavigationSettings.spotCnt = 0;                            
                        }
                    }
                    target = settings.activeNavigationStructure.toGoWhenNoMobs[botNavigationSettings.spotCnt].pos;
                    botNavigationSettings.bypassZoneReached = true;
                    state = NavigationState::CHANGE_SPOT;

                    std::cout<<"[LOG][NAVIGATION][" << utils::getTimestamp() << "] Bypass zone already reached or no bypass zone, going to spot number "<<botNavigationSettings.spotCnt<<"\n";
                }
            } break;
        }

        prevCharPos=charPos;
        botNavigationSettings.movementRateCnt=0;
    }
    botNavigationSettings.movementRateCnt++;

    return false;
}

void BotNavigation::processGoToBypassZone()
{

}
void BotNavigation::processChangeSpot()
{

}
void BotNavigation::processOutOfAttackingArea()
{
    
}

void BotNavigation::goToAttackingArea()
{
    float minDist = movement::getDistance(charPosImg, settings.activeNavigationStructure.trajectory[0].pos);
    for (size_t i = 1; i < settings.activeNavigationStructure.trajectory.size(); ++i)
    {
        float d = movement::getDistance(charPosImg, settings.activeNavigationStructure.trajectory[i].pos);
        if (d < minDist)
        {
            minDist = d;
            botNavigationSettings.trajectoryCnt = i;
        }
    }

    float d = movement::getDistance(charPosImg, settings.activeNavigationStructure.attackingArea[0].pos);
    if (d < minDist)
    {
        target = settings.activeNavigationStructure.attackingArea[0].pos;
    }
    else
    {
        target = settings.activeNavigationStructure.trajectory[botNavigationSettings.trajectoryCnt].pos;
    }
    state = NavigationState::OUT_OF_ATTACKING_AREA;

    std::cout<<"[LOG][NAVIGATION][" << utils::getTimestamp() << "] Out of attacking area, trying to return.\n";
}

void BotNavigation::changeSpot()
{
    state = NavigationState::GO_TO_BYPASS_ZONE;

    if (settings.activeNavigationStructure.bypass.size()>0)
        target = settings.activeNavigationStructure.bypass[0].pos;
    else
        target = settings.activeNavigationStructure.toGoWhenNoMobs[0].pos;
}

bool BotNavigation::isInsideAttackingArea()
{
    return movement::isInsideCircle(
        settings.activeNavigationStructure.attackingArea[0].pos.first, 
        settings.activeNavigationStructure.attackingArea[0].pos.second, 
        charPosImg.first, 
        charPosImg.second, settings.activeNavigationStructure.attackingArea[0].radius);
}

bool BotNavigation::isInsideCurrentSpot()
{
    return movement::isInsideCircle(
        settings.activeNavigationStructure.toGoWhenNoMobs[botNavigationSettings.spotCnt].pos.first, 
        settings.activeNavigationStructure.toGoWhenNoMobs[botNavigationSettings.spotCnt].pos.second, 
        charPosImg.first, 
        charPosImg.second, settings.activeNavigationStructure.toGoWhenNoMobs[botNavigationSettings.spotCnt].radius);
}

void BotNavigation::setState(NavigationState nState)
{
    state = nState;
}

NavigationState BotNavigation::getState()
{
    return state;
}