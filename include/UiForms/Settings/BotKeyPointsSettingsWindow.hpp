#pragma once

#include <QWidget>
#include <QPushButton>
#include <QDebug>
#include <QScrollArea>

#include "IO/WindowManager.hpp"
#include "UiForms/PixelPickerDialog.hpp"
#include "Bot/SettingsManager.hpp"
#include "CustomUiObj/SettingsWindowRgbPosWidget.hpp"
#include "types.hpp"
#include "interfaces.hpp"

class BotKeyPointsSettingsWindow : public QWidget, public ICustomView {
    Q_OBJECT
public:
    explicit BotKeyPointsSettingsWindow(std::shared_ptr<WindowManager> wm, QWidget* parent = nullptr);
private slots:
    void onSaveClicked();

private:
    void setupUi();
    void setupLayout();
    void setupConnections();

    std::shared_ptr<WindowManager> wm;
    SettingsManager settingsManager;
    BotSettings settings;

    QWidget* container;
    QScrollArea* scrollArea;
    QVBoxLayout* layout;

    SettingsWindowRgbPosWidget* mobHealthWidget;
    SettingsWindowRgbPosWidget* isPartyMobWidget;
    SettingsWindowRgbPosWidget* isNormalMobWidget;
    SettingsWindowRgbPosWidget* isChampionMobWidget;
    SettingsWindowRgbPosWidget* isGiantMobWidget;
    SettingsWindowRgbPosWidget* skill1Widget;
    SettingsWindowRgbPosWidget* skill2Widget;
    SettingsWindowRgbPosWidget* skill3Widget;
    SettingsWindowRgbPosWidget* skill4Widget;
    SettingsWindowRgbPosWidget* skill5Widget;
    SettingsWindowRgbPosWidget* buff1Widget;
    SettingsWindowRgbPosWidget* buff2Widget;
    SettingsWindowRgbPosWidget* buff3Widget;
    QPushButton* saveButton;
};
