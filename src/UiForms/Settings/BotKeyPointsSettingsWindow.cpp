#include "UiForms/Settings/BotKeyPointsSettingsWindow.hpp"

BotKeyPointsSettingsWindow::BotKeyPointsSettingsWindow(std::shared_ptr<WindowManager> wm, QWidget* parent)
        : QWidget(parent), wm(wm), settingsManager()
{
    setWindowTitle("Settings Window");
    setFixedSize(230, 300);
    setAttribute(Qt::WA_DeleteOnClose);

    settings = settingsManager.loadSettings("assets/settings.json");
}

void BotKeyPointsSettingsWindow::setupUi()
{
    saveButton = new QPushButton("Save", this);
    saveButton->setGeometry(0, 300-40, 230, 40);

    scrollArea = new QScrollArea(this);
    scrollArea->setGeometry(0, 0, 230, 300-40);
    scrollArea->setWidgetResizable(true);

    mobHealthWidget = new SettingsWindowRgbPosWidget("Mob Health", wm, settings.mobHealthPixel, this);
    isPartyMobWidget = new SettingsWindowRgbPosWidget("Is Party Mob", wm, settings.isPartyMobPixel, this);
    isNormalMobWidget = new SettingsWindowRgbPosWidget("Is Normal Mob", wm, settings.isNormalMobPixel, this);
    isChampionMobWidget = new SettingsWindowRgbPosWidget("Is Champion Mob", wm, settings.isChampionMobPixel, this);
    isGiantMobWidget = new SettingsWindowRgbPosWidget("Is Giant Mob", wm, settings.isGiantMobPixel, this);
    skill1Widget = new SettingsWindowRgbPosWidget("Skill 1", wm, settings.Skill1Pixel, this);
    skill2Widget = new SettingsWindowRgbPosWidget("Skill 2", wm, settings.Skill2Pixel, this);
    skill3Widget = new SettingsWindowRgbPosWidget("Skill 3", wm, settings.Skill3Pixel, this);
    skill4Widget = new SettingsWindowRgbPosWidget("Skill 4", wm, settings.Skill4Pixel, this);
    skill5Widget = new SettingsWindowRgbPosWidget("Skill 5", wm, settings.Skill5Pixel, this);
    buff1Widget = new SettingsWindowRgbPosWidget("Buff 1", wm, settings.Buff1Pixel, this);
    buff2Widget = new SettingsWindowRgbPosWidget("Buff 2", wm, settings.Buff2Pixel, this);
    buff3Widget = new SettingsWindowRgbPosWidget("Buff 3", wm, settings.Buff3Pixel, this);
}
void BotKeyPointsSettingsWindow::setupLayout()
{
    container = new QWidget();
    layout = new QVBoxLayout(container);
    layout->setSpacing(5);
    layout->setContentsMargins(0,0,0,0);
    
    layout->addWidget(mobHealthWidget);
    layout->addWidget(isPartyMobWidget);
    layout->addWidget(isNormalMobWidget);
    layout->addWidget(isChampionMobWidget);
    layout->addWidget(isGiantMobWidget);
    layout->addWidget(skill1Widget);
    layout->addWidget(skill2Widget);
    layout->addWidget(skill3Widget);
    layout->addWidget(skill4Widget);
    layout->addWidget(skill5Widget);
    layout->addWidget(buff1Widget);
    layout->addWidget(buff2Widget);
    layout->addWidget(buff3Widget);
    
    layout->addStretch();
    scrollArea->setWidget(container);
}

void BotKeyPointsSettingsWindow::setupConnections()
{
    connect(saveButton, &QPushButton::clicked, this, &BotKeyPointsSettingsWindow::onSaveClicked);
}

void BotKeyPointsSettingsWindow::onSaveClicked()
{
    settings.mobHealthPixel = mobHealthWidget->getResult();
    settings.Skill1Pixel = skill1Widget->getResult();
    settings.Skill2Pixel = skill2Widget->getResult();
    settings.Skill3Pixel = skill3Widget->getResult();
    settings.Skill4Pixel = skill4Widget->getResult();
    settings.Skill5Pixel = skill5Widget->getResult();
    settings.Buff1Pixel = buff1Widget->getResult();
    settings.Buff2Pixel = buff2Widget->getResult();
    settings.Buff3Pixel = buff3Widget->getResult();
    
    settingsManager.saveSettings(settings, "assets/settings.json");
    this->close();
}