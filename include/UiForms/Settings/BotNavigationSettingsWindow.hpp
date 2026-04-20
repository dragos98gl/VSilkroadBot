#pragma once

#include <QWidget>
#include <QDialog>
#include <QRadioButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QMenu>
#include <QPushButton>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>

#include "CustomUiObj/BotNavigationMapView.hpp"
#include "IO/WindowManager.hpp"
#include "Bot/CharacterMovement.hpp"
#include "types.hpp"
#include "interfaces.hpp"
#include "Bot/SettingsManager.hpp"

class BotNavigationSettingsWindow : public QWidget, public ICustomView
{
    Q_OBJECT

public:
    explicit BotNavigationSettingsWindow(
        std::shared_ptr<WindowManager> wm,
        std::shared_ptr<cv::dnn::Net>  digitsNet,
        QWidget* parent = nullptr
    );

private slots:
    void onCircleListChanged(const MapCircle c);
    void onNewNavigation();
    void onNavigationChanged(int index);
    void onSave();
    void onUseCharPos();
    void onRadiusChanged(const QString& text);

private:
    // ── Setup ─────────────────────────────────────────────────────────────
    void setupUi();
    void setupLayout();
    void setupConnections();

    // ── Helpers ───────────────────────────────────────────────────────────
    void populateScrollContent();
    void addPointItem(const QString& name, const QPoint& pos, NavigationType type, int index);

    NavigationStructure* currentZone() const;
    NavigationType       selectedNavigationType() const;
    float                defaultRadiusFor(NavigationType type) const;
    void                 addElementToCurrentZone(NavigationElement e);
    void                 loadMapAtCharPos();
    void                 loadMapAtZoneOrigin();

    // ── Dependencies ──────────────────────────────────────────────────────
    std::shared_ptr<WindowManager> wm;
    std::shared_ptr<cv::dnn::Net>  digitsNet;

    // ── Data ─────────────────────────────────────────────────────────────
    BotSettings settings;
    int         currentZoneIndex = -1;  // replaces raw NavigationStructure* pointer

    static constexpr const char* MAP_PATH      = "assets/map.png";
    static constexpr int         MAP_VIEW_HALF = 150;

    // ── Widgets ───────────────────────────────────────────────────────────
    BotNavigationMapView* botNavigationMapView;

    QLabel*    activeNavigationLabel;
    QComboBox* activeNavigationComboBox;

    QButtonGroup* selectionType;
    QRadioButton* attackingArea;
    QRadioButton* toGoWhenNoMobs;
    QRadioButton* trajectory;
    QRadioButton* bypass;

    QPushButton* useCharPosButton;
    QPushButton* saveButton;
    QPushButton* newButton;

    QLabel*    radiusLabel;
    QLineEdit* radiusLineEdit;

    QVBoxLayout* pointsLayout;
};