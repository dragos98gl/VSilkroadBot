#include "UiForms/Settings/BotCombatSettingsWindow.hpp"

BotCombatSettingsWindow::BotCombatSettingsWindow(QWidget* parent)
    : QWidget(parent), settingsManager()
{
    setWindowTitle("Combat Settings");
    setFixedWidth(360);
    setAttribute(Qt::WA_DeleteOnClose);

    settings = settingsManager.loadSettings("assets/settings.json");

    setupUi();
    setupLayout();
    setupConnections();
    
    adjustSize();
}

void BotCombatSettingsWindow::setupUi()
{
    waitForMobSpin                      = createSpinBox(0, 10000, settings.waitForMobValue.count());
    waitForHealthBarSpin                = createSpinBox(0, 10000, settings.waitForHealthBarValue.count());
    noMobsFoundSpin                     = createSpinBox(0, 100, settings.noMobsFoundThreshold);
    mobsSeekLimitSpin                   = createSpinBox(0, 100, settings.mobsSeekLimit);
    mobsSelectionFailureThresholdSpin   = createSpinBox(0, 100, settings.mobsSelectionFailureThreshold);

    normalMobTimeoutSpin                = createSpinBox(1, 300, settings.normalMobTimeout);
    championMobTimeoutSpin              = createSpinBox(1, 300, settings.championMobTimeout);
    giantMobTimeoutSpin                 = createSpinBox(1, 300, settings.giantMobTimeout);

    doubleClickCheck  = new QCheckBox();
    doubleClickCheck->setChecked(settings.doubleClickOnMob);

    saveButton = new QPushButton("Save");
}

void BotCombatSettingsWindow::setupLayout()
{
    mainLayout = new QVBoxLayout(this);
    gridLayout = new QGridLayout();

    int row = 0;

    addRow(gridLayout, row++, "Wait for mob (ms)", waitForMobSpin);
    addRow(gridLayout, row++, "Wait for HP bar (ms)", waitForHealthBarSpin);
    addRow(gridLayout, row++, "Double click mob", doubleClickCheck);
    addRow(gridLayout, row++, "No mobs threshold", noMobsFoundSpin);
    addRow(gridLayout, row++, "Mobs seek limit", mobsSeekLimitSpin);
    addRow(gridLayout, row++, "Mobs selection failure threshold", mobsSelectionFailureThresholdSpin);
    
    addRow(gridLayout, row++, "Normal mob timeout (s)", normalMobTimeoutSpin);
    addRow(gridLayout, row++, "Champion mob timeout (s)", championMobTimeoutSpin);
    addRow(gridLayout, row++, "Giant mob timeout (s)", giantMobTimeoutSpin);

    mainLayout->addLayout(gridLayout);
    mainLayout->addWidget(saveButton);
}

void BotCombatSettingsWindow::setupConnections()
{    
    connect(saveButton, &QPushButton::clicked, this, &BotCombatSettingsWindow::onSaveClicked);
}

QSpinBox* BotCombatSettingsWindow::createSpinBox(int min, int max, int value)
{
    auto* box = new QSpinBox();
    box->setRange(min, max);
    box->setValue(value);
    return box;
}

void BotCombatSettingsWindow::addRow(QGridLayout* layout, int row,
            const QString& labelText, QWidget* widget)
{
    layout->addWidget(new QLabel(labelText), row, 0);
    layout->addWidget(widget, row, 1);
}

void BotCombatSettingsWindow::onSaveClicked()
{
    settings.waitForMobValue = std::chrono::milliseconds(waitForMobSpin->value());
    settings.waitForHealthBarValue = std::chrono::milliseconds(waitForHealthBarSpin->value());
    settings.noMobsFoundThreshold = noMobsFoundSpin->value();
    settings.mobsSeekLimit = mobsSeekLimitSpin->value();
    settings.mobsSelectionFailureThreshold = mobsSelectionFailureThresholdSpin->value();
    settings.doubleClickOnMob = doubleClickCheck->isChecked();

    settings.normalMobTimeout = normalMobTimeoutSpin->value();
    settings.championMobTimeout = championMobTimeoutSpin->value();
    settings.giantMobTimeout = giantMobTimeoutSpin->value();
    
    settingsManager.saveSettings(settings, "assets/settings.json");
    this->close();
}