#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>

#include <memory>
#include <opencv2/dnn.hpp>

#include "IO/WindowManager.hpp"
#include "Bot/SettingsManager.hpp"

class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onModelSelectionChanged();
    void onCheckHandleClicked();
    void onTrainNewModelClicked();
    void onModifyExistingModelClicked();
    void onBotOptionsClicked();
    void onStartBotClicked();
    void onTrainClicked();
    void onLoadModelClicked();

private:
    void setupUI();
    void populateModelsList();
    void connectSignals();

    std::shared_ptr<WindowManager>  wm;
    std::shared_ptr<cv::dnn::Net>   mobsNet;
    std::shared_ptr<cv::dnn::Net>   digitsNet;

    SettingsManager settingsManager;
    BotSettings settings;
    
    QLineEdit*    clientWindowName;
    QPushButton*  checkHandle;
    QPushButton*  trainNewModelButton;
    QPushButton*  loadModelButton;
    QPushButton*  modifyExistingModelButton;
    QPushButton*  trainButton;
    QListWidget*  modelsList;
    QPushButton*  botOptionsButton;
    QPushButton*  startBotButton;
};