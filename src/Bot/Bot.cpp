#include <windows.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include <QDebug>

#include <string>
#include <filesystem>

#include "Bot/Bot.hpp"
#include "Bot/CharacterMovement.hpp"

#include "utils.hpp"

namespace fs = std::filesystem;

Bot::Bot(
    std::shared_ptr<WindowManager> windowManager,
    std::shared_ptr<cv::dnn::Net> mobsNet,
    std::shared_ptr<cv::dnn::Net> digitsNet) : 
        hidController(),
        wm(windowManager),
        mobsNet(mobsNet),
        digitsNet(digitsNet),
        botNavigation(hidController,wm),
        nearbyPlayerDebouncer(5, 5, 10.0)
{    
    SettingsManager settingsManager;
    settings = settingsManager.loadSettings("assets/settings.json");

    map = cv::imread("assets/map.png", cv::IMREAD_COLOR);
    
    frame_process::drawNavigationElements(map,settings.activeNavigationStructure);

    nearbyPlayerDebouncer.setOnDetected([]() {
        std::cout << "[LOG][WARNING][" << utils::getTimestamp() << "] Players detected nearby!\n";
    });

    nearbyPlayerDebouncer.setOnCleared([]() {
        std::cout << "[LOG][WARNING][" << utils::getTimestamp() << "] Players left the area.\n";
    });
}

Bot::~Bot()
{
    stop();
}

void Bot::start()
{
    if (running)
        return;

    running = true;
    runThread = std::thread(&Bot::run, this);
}

void Bot::onSettingsUpdate(bool onlyAttacking)
{
    this->onlyAttacking = onlyAttacking;
}


void Bot::run()
{
    state=BotState::SearchingForMob;

    if (onlyAttacking)
        state=BotState::AttackingMob;

    while (running)
    {
        screenRGB = wm->getRgbScreenshot();
        windowInfo = wm->getWindowInfo();

        charPos = movement::getCharPos(screenRGB,digitsNet);
        charPosImg = movement::convertToImgCoordinates(charPos);

        botNavigation.updatePosition(charPos);
        
        /*CHECK IF PLAYERS ARE NEARBY*/
        cv::Mat minimap = frame_process::getMinimap(screenRGB);
        bool playersNearby = frame_process::arePlayersNearby(minimap);
        nearbyPlayerDebouncer.update(playersNearby);
        /*CHECK IF PLAYERS ARE NEARBY*/

        /*MOBS DETECTION*/
        std::vector<NetOutput> netOut = frame_process::mobsNN::searchForMob(screenRGB,mobsNet.get());
        /*MOBS DETECTION*/
        
        /*UPDATE BOT UI*/
        cv::Mat detections = frame_process::drawDetectedMobsRect(screenRGB, netOut);
        cv::resize(detections, detections, cv::Size(320, 180));
        emit updateBotUI(
            detections, 
            map, 
            charPosImg, 
            settings.activeNavigationStructure.attackingArea[0].pos, 
            settings.activeNavigationStructure.attackingArea[0].radius, 
            botNavigation.isInsideAttackingArea());
        /*UPDATE BOT UI*/

        /*CHECK IF PLAYER IS OUT OF ATTACKING AREA*/
        if (
            !onlyAttacking && 
            botNavigation.getState()!=NavigationState::OUT_OF_ATTACKING_AREA && 
            !botNavigation.isInsideAttackingArea() &&
            !frame_process::comparePixels(screenRGB.at<cv::Vec3b>(settings.mobHealthPixel.position.y(), settings.mobHealthPixel.position.x()), settings.mobHealthPixel.color))
        {
            state = BotState::Movement;
            botNavigation.goToAttackingArea();
        }
        /*CHECK IF PLAYER IS OUT OF ATTACKING AREA*/

        /*CHECK IF CHANGING SPOT IS REQUIRED*/
        bool isChangingSpotRequired = !onlyAttacking && 
            state != BotState::Movement &&
            noMobsFoundCnt>settings.noMobsFoundThreshold && 
            !frame_process::comparePixels(screenRGB.at<cv::Vec3b>(settings.mobHealthPixel.position.y(), settings.mobHealthPixel.position.x()), settings.mobHealthPixel.color);
        
        if (isChangingSpotRequired)
        {
            std::cout<<"[LOG][NAVIGATION][" << utils::getTimestamp() << "] No mobs found, changing spot area.\n";
            state = BotState::Movement;
            noMobsFoundCnt = 0;
            botNavigation.changeSpot();
        }
        /*CHECK IF CHANGING SPOT IS REQUIRED*/

        switch (state)
        {
            case BotState::Movement:
            {
                processMovement();
            } break;
            case BotState::SearchingForMob:
            {
                processSearchingForMob(netOut);
            } break;
            case BotState::AttackingMob:
            {
                processAttackingMob();
            } break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Bot::processMovement()
{
    if (botNavigation.run() || onlyAttacking)
    {
        state=BotState::SearchingForMob;
    }
}

void Bot::processSearchingForMob(std::vector<NetOutput>& netOut)
{
    /*IF MOB HEALTH BAR IS ALREADY FOUND SKIP SORTING AND CHECKING*/
    if (frame_process::comparePixels(screenRGB.at<cv::Vec3b>(settings.mobHealthPixel.position.y(), settings.mobHealthPixel.position.x()), settings.mobHealthPixel.color))
    {
        noMobsFoundCnt = 0;
        state = BotState::AttackingMob;
        return;
    }

    /*SORT DETECTIONS BY A COMBINED SCORE OF SIZE AND DISTANCE TO CENTER*/
    const float w_size = 0.1f;
    const float w_dist = 0.9f;

    int cx = screenRGB.cols / 2;
    int cy = screenRGB.rows / 2;

    float maxDist = cx * cx + cy * cy;
    float maxSize = screenRGB.cols * screenRGB.rows;

    std::sort(netOut.begin(), netOut.end(),
        [&](const NetOutput& a, const NetOutput& b)
        {
            auto score = [&](const NetOutput& o)
            {
                float size = o.box.width * o.box.height;
                float dist = (o.center.x - cx) * (o.center.x - cx) +
                            (o.center.y - cy) * (o.center.y - cy);

                float sizeNorm = size / maxSize;
                float distNorm = dist / maxDist;

                return w_size * sizeNorm - w_dist * distNorm;
            };

            return score(a) > score(b);
        });
    /*SORT DETECTIONS BY A COMBINED SCORE OF SIZE AND DISTANCE TO CENTER*/
    
    for (auto& detection : netOut)
    {
        /*LIMIT THE NUMBER OF MOBS TO CHECK*/
        mobsSeekLimitCnt = 0;
        if (mobsSeekLimitCnt >= settings.mobsSeekLimit || state == BotState::AttackingMob) break;
        mobsSeekLimitCnt++;

        hidController.setMousePosition(frame_process::positionToWindow(windowInfo.windowPosition, detection.center,windowInfo.screenResolution));

        /*CHECK IF MOUSE CURSOR IS CHANGING*/
        if (waitForMob(settings.waitForMobValue))
        {
            hidController.leftClickMouse(frame_process::positionToWindow(windowInfo.windowPosition, detection.center,windowInfo.screenResolution));
            if (settings.doubleClickOnMob)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                hidController.leftClickMouse(frame_process::positionToWindow(windowInfo.windowPosition, detection.center,windowInfo.screenResolution));
            }

            /*PIXEL CHECK FOR MOB HEALTH BAR*/
            bool mobHealthBarFound = utils::waitUntil(
                [&]() { 
                    screenRGB = wm->getRgbScreenshot();
                    return frame_process::comparePixels(screenRGB.at<cv::Vec3b>(settings.mobHealthPixel.position.y(), settings.mobHealthPixel.position.x()), settings.mobHealthPixel.color);  
                },
                settings.waitForHealthBarValue,
                1ms
            );

            if (mobHealthBarFound)
            {
                noMobsFoundCnt = 0;
                state = BotState::AttackingMob;
                break;
            }
        }
    }

    noMobsFoundCnt++;
}

void Bot::processAttackingMob()
{
    MobType detectedMob = MobType::None;
    /*CHECK IF THE MOB HEALTH BAR DISAPPEARD*/
    if (!frame_process::comparePixels(screenRGB.at<cv::Vec3b>(settings.mobHealthPixel.position.y(), settings.mobHealthPixel.position.x()), settings.mobHealthPixel.color))
    {
        state = BotState::SearchingForMob;
        hidController.setMousePosition(frame_process::positionToWindow(windowInfo.windowPosition, std::make_pair(10, 10),windowInfo.screenResolution));
        mobsSelectionFailureCnt = 0;

        detectedMob = MobType::None;
        mobTimer.reset();

        return;
    }

    if(frame_process::comparePixels(screenRGB.at<cv::Vec3b>(settings.isGiantMobPixel.position.y(), settings.isGiantMobPixel.position.x()), settings.isGiantMobPixel.color))
    {
        detectedMob = MobType::Giant;
    }  
    if(frame_process::comparePixels(screenRGB.at<cv::Vec3b>(settings.isChampionMobPixel.position.y(), settings.isChampionMobPixel.position.x()), settings.isChampionMobPixel.color))
    {
        detectedMob = MobType::Champion;
    } 
    if(frame_process::comparePixels(screenRGB.at<cv::Vec3b>(settings.isNormalMobPixel.position.y(), settings.isNormalMobPixel.position.x()), settings.isNormalMobPixel.color))
    {
        detectedMob = MobType::Normal;
    }  

    mobTimer.update(detectedMob);
    if (mobTimer.isExpired())
    {
        std::cout<<"[LOG][COMBAT][" << utils::getTimestamp() << "] Mob timer expired, resetting.\n";
        detectedMob = MobType::None;
        mobTimer.reset();

        if(mobsSelectionFailureCnt>=settings.mobsSelectionFailureThreshold)
        {
            std::cout<<"[LOG][COMBAT][" << utils::getTimestamp() << "] Mob selection failure threshold reached, changing spot.\n";
            botNavigation.changeSpot();
            mobsSelectionFailureCnt = 0;
        } else
        {
            state = BotState::SearchingForMob;
            hidController.setMousePosition(frame_process::positionToWindow(windowInfo.windowPosition, std::make_pair(10, 10),windowInfo.screenResolution));
        }

        mobsSelectionFailureCnt++;
    }

    /*SKILLS EXECUTION*/
    std::vector<std::reference_wrapper<const decltype(settings.Skill1Pixel.position)>> skillPositions = {
        settings.Skill1Pixel.position,
        settings.Skill2Pixel.position,
        settings.Skill3Pixel.position,
        settings.Skill4Pixel.position,
        settings.Skill5Pixel.position
    };

    auto executeSkill = [&](const auto& pixelPos)
    {
        auto windowPos = frame_process::positionToWindow(
            windowInfo.windowPosition,
            pixelPos,
            windowInfo.screenResolution);

        hidController.rightClickMouse(windowPos);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        hidController.rightClickMouse(windowPos);
    };

    if (skillIndex < skillPositions.size())
    {
        executeSkill(skillPositions[skillIndex]);
    }

    skillIndex = (skillIndex + 1) % skillPositions.size();
    /*SKILLS EXECUTION*/

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void Bot::stop()
{
    running = false;

    if (runThread.joinable())
        runThread.join();
}

bool Bot::waitForMob(std::chrono::milliseconds timeout)
{
    return utils::waitUntil(
        [this]() { 
            return frame_process::checkIfMob(wm); 
        },
        timeout,
        1ms
    );
}