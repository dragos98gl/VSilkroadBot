#pragma once

#include <opencv2/opencv.hpp>

#include <QColor>
#include <QPoint>

using namespace std::chrono_literals;

enum class NavigationState
{
    OUT_OF_ATTACKING_AREA,
    CHANGE_SPOT,
    GO_TO_BYPASS_ZONE,
    NONE
};

enum class BotState
{
    SearchingForMob,
    AttackingMob,
    Movement,
    None
};

struct NetOutput {
    cv::Rect box;
    float confidence;
    cv::Point center;
};

struct WindowInfo
{
    std::pair<int,int> windowPosition;
    std::pair<int,int> windowSize;
    std::pair<int,int> windowCenter;
    std::pair<int,int> screenResolution;
};

struct PixelInfo
{
    QPoint position;
    QColor color;
};

struct TrainingAsset
{
    cv::Mat img;
    std::vector<QRect> labels;
};

struct TrainingModel
{
    std::string imgPath;
    std::string labelsPath;
    std::string name;
    TrainingAsset asset;
};

enum class NavigationType
{
    ATTACKING_AREA, //(0, 0, 255, 255 * 0.3)
    TO_GO_WHEN_NO_MOBS, //(0, 255, 0, 255 * 0.3)
    TRAJECTORY, //(255, 0, 0, 255 * 1)
    BYPASS //(255, 255, 255, 255 * 0.3)
};

struct NavigationElement
{
    std::pair<int,int> pos;
    NavigationType type;
    float radius;
};

struct NavigationStructure
{
    std::string name;
    bool isActive; 

    std::vector<NavigationElement> attackingArea;
    std::vector<NavigationElement> toGoWhenNoMobs;
    std::vector<NavigationElement> trajectory;
    std::vector<NavigationElement> bypass;
};

struct BotSettings
{
    PixelInfo mobHealthPixel;
    PixelInfo isEliteMobPixel;
    PixelInfo isPartyMobPixel;
    PixelInfo isNormalMobPixel;
    PixelInfo isChampionMobPixel;
    PixelInfo isGiantMobPixel;
    PixelInfo Skill1Pixel;
    PixelInfo Skill2Pixel;
    PixelInfo Skill3Pixel;
    PixelInfo Skill4Pixel;
    PixelInfo Skill5Pixel;
    PixelInfo Buff1Pixel;
    PixelInfo Buff2Pixel;
    PixelInfo Buff3Pixel;
    
    std::chrono::milliseconds waitForMobValue;
    std::chrono::milliseconds waitForHealthBarValue;
    bool doubleClickOnMob = false;
    int noMobsFoundThreshold;
    int mobsSelectionFailureThreshold;
    int mobsSeekLimit;
    
    int normalMobTimeout;
    int championMobTimeout;
    int giantMobTimeout;

    std::string windowName;

    std::vector<NavigationStructure> navigationStructure;
    NavigationStructure activeNavigationStructure;
};

enum class MobType
{
    None,
    Normal,
    Elite,
    Champion,
    Giant
};

struct MapCircle
{
    NavigationElement nElement;
    QColor color;
};
