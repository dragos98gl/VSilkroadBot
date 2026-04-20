#pragma once

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QMutex>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDebug>

#include <opencv2/opencv.hpp>

#include "Bot/Bot.hpp"
#include "Bot/FrameProcess.hpp"
#include "CustomUiObj/DoubleImageView.hpp"
#include "interfaces.hpp"

class BotUi : public QWidget, public ICustomView
{
    Q_OBJECT
public:
    explicit BotUi(
            std::shared_ptr<WindowManager> windowManager,
            std::shared_ptr<cv::dnn::Net> mobsNet,
            std::shared_ptr<cv::dnn::Net> digitsNet,
            QWidget* parent = nullptr);

public slots:
    void onUpdateBotUI(
        const cv::Mat& mobsDetection, 
        const cv::Mat& map, 
        const std::pair<int, int>& charPosImg, 
        const std::pair<int, int>& attackingAreaPos,
        float attackingAreaRadius,
        bool isInsideAttackingArea);
    void onOnlyAttackingToggled();

signals:
    void settingsUpdated(bool onlyAttacking);

private:
    QImage matToQImage(const cv::Mat& img, int targetW, int targetH);
    void setupUi() override;
    void setupLayout() override;
    void setupConnections() override;

    Bot bot;

    DoubleImageView* doubleImageView;

    QVBoxLayout* mainLayout;
    QCheckBox* onlyAttackingCheck;
};