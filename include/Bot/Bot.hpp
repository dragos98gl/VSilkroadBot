#pragma once

#include <cstdint>
#include <thread>
#include <atomic>

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <QWidget>

#include "MobTimer.hpp"
#include "IO/WindowManager.hpp"
#include "IO/HIDController.hpp"
#include "types.hpp"
#include "BotNavigation.hpp"
#include "Bot/FrameProcess.hpp"
#include "Consts.hpp"
#include "Bot/NearbyPlayerDebouncer.hpp"

//(6434x928) (3285x498)        (-11161x2646) (350x209)
//(6434x928) (3926x551)        (-11161x2646) (988x265) 
using namespace std::chrono_literals;
class Bot : public QWidget
{
Q_OBJECT
public:
    Bot(
        std::shared_ptr<WindowManager> windowManager,
        std::shared_ptr<cv::dnn::Net> mobsNet, 
        std::shared_ptr<cv::dnn::Net> digitsNet);
    ~Bot();
    
    void start();
    void stop();

signals:
    void updateBotUI(
        const cv::Mat& mobsDetection, 
        const cv::Mat& map, 
        const std::pair<int, int>& charPosImg, 
        const std::pair<int, int>& attackingAreaPos,
        float attackingAreaRadius,
        bool isInsideAttackingArea 
        );

public slots:
    void onSettingsUpdate(bool onlyAttacking);
private:
    void run();
    bool waitForMob(std::chrono::milliseconds timeout);
    void processMovement();
    void processSearchingForMob(std::vector<NetOutput>& netOut);
    void processAttackingMob();

    HIDController hidController;
    std::shared_ptr<WindowManager> wm;
    std::shared_ptr<cv::dnn::Net> mobsNet;
    std::shared_ptr<cv::dnn::Net> digitsNet;

    BotSettings settings;
    BotNavigation botNavigation;
    MobTimer mobTimer;
    NearbyPlayerDebouncer nearbyPlayerDebouncer;

    std::thread runThread;
    std::atomic<bool> running{false};

    
    cv::Mat screenRGB;
    WindowInfo windowInfo;
    std::pair<float,float> charPos;
    std::pair<float,float> charPosImg;
    cv::Mat map;
    BotState state {BotState::SearchingForMob};
    int skillIndex = 0;
    
    bool onlyAttacking = false;
    int mobsSelectionFailureCnt = 0;
    int noMobsFoundCnt=0;
    int mobsSeekLimitCnt=0;
};