#include "UiForms/MainWindow/BotSettingsMainWindow.hpp"

BotSettingsMainWindow::BotSettingsMainWindow(
    std::shared_ptr<WindowManager> wm,
    std::shared_ptr<cv::dnn::Net> digitsNet, 
    QWidget* parent)
        : QWidget(parent), wm(wm), digitsNet(digitsNet)
{
    setupUi();
    setupLayout();
    setupConnections();
}

void BotSettingsMainWindow::setupUi()
{
    botNavigationSettings = new QPushButton("Bot Navigation Settings", this);
    botNavigationSettings->setGeometry(0, 0, 230, 40);

    botKeyPointsSettings = new QPushButton("Bot Key Points Settings", this);
    botKeyPointsSettings->setGeometry(0, 40, 230, 40);

    botCombatSettings = new QPushButton("Bot Combat Settings", this);
    botCombatSettings->setGeometry(0, 80, 230, 40);
}

void BotSettingsMainWindow::setupLayout()
{
    setWindowTitle("Settings Window");
    setFixedSize(230, 120);
    setAttribute(Qt::WA_DeleteOnClose);
}

void BotSettingsMainWindow::setupConnections()
{
    connect(botNavigationSettings, &QPushButton::clicked, this, &BotSettingsMainWindow::onBotBotNavigationSettingsClicked);
    connect(botKeyPointsSettings, &QPushButton::clicked, this, &BotSettingsMainWindow::onBotKeyPointsSettingsClicked);
    connect(botCombatSettings, &QPushButton::clicked, this, &BotSettingsMainWindow::onBotCombatSettingsClicked);
}

void BotSettingsMainWindow::onBotBotNavigationSettingsClicked()
{
    auto* botNavigationSettingsWindow = new BotNavigationSettingsWindow(wm,digitsNet);
    botNavigationSettingsWindow->show();
}
void BotSettingsMainWindow::onBotKeyPointsSettingsClicked()
{
    auto* botKeyPointsSettingsWindow = new BotKeyPointsSettingsWindow(wm);
    botKeyPointsSettingsWindow->show();
}
void BotSettingsMainWindow::onBotCombatSettingsClicked()
{
    auto* botCombatSettingsWindow = new BotCombatSettingsWindow();
    botCombatSettingsWindow->show();
}