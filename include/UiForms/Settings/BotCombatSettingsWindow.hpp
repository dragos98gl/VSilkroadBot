#pragma once

#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>

#include "interfaces.hpp"
#include "Bot/SettingsManager.hpp"

class BotCombatSettingsWindow : public QWidget, public ICustomView
{
    Q_OBJECT
public:
    explicit BotCombatSettingsWindow(QWidget* parent = nullptr);
    
private slots:
    void onSaveClicked();

private:
    void setupUi() override;
    void setupLayout() override;
    void setupConnections() override;

    QSpinBox* createSpinBox(int min, int max, int value);
    void addRow(QGridLayout* layout, int row,
                const QString& labelText, QWidget* widget);

    SettingsManager settingsManager;
    BotSettings settings;
    
    QSpinBox* waitForMobSpin;
    QSpinBox* waitForHealthBarSpin;
    QSpinBox* noMobsFoundSpin;
    QSpinBox* mobsSeekLimitSpin;
    QSpinBox* mobsSelectionFailureThresholdSpin;
    QCheckBox* doubleClickCheck;

    QSpinBox* normalMobTimeoutSpin;
    QSpinBox* championMobTimeoutSpin;
    QSpinBox* giantMobTimeoutSpin;

    QPushButton* saveButton;

    QVBoxLayout* mainLayout;
    QGridLayout* gridLayout;
};