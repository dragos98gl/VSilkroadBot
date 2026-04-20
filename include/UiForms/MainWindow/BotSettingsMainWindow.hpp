#pragma once

#include <QWidget>
#include <QPushButton>

#include "IO/WindowManager.hpp"
#include "UiForms/Settings/BotKeyPointsSettingsWindow.hpp"
#include "UiForms/Settings/BotNavigationSettingsWindow.hpp"
#include "UiForms/Settings/BotCombatSettingsWindow.hpp"
#include "interfaces.hpp"

class BotSettingsMainWindow : public QWidget, public ICustomView {
    Q_OBJECT
public:
    explicit BotSettingsMainWindow(
        std::shared_ptr<WindowManager> wm,
        std::shared_ptr<cv::dnn::Net> digitsNet, 
        QWidget* parent = nullptr);
private:
    void setupUi() override;
    void setupLayout() override;
    void setupConnections() override;

    void onBotBotNavigationSettingsClicked();
    void onBotKeyPointsSettingsClicked();
    void onBotCombatSettingsClicked();

    std::shared_ptr<WindowManager> wm;
    std::shared_ptr<cv::dnn::Net> digitsNet;

    QPushButton* botNavigationSettings;
    QPushButton* botKeyPointsSettings;
    QPushButton* botCombatSettings;
};
