#include "UiForms/MainWindow/MainWindow.hpp"

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

#include <windows.h>

#include "UiForms/MainWindow/TrainWindow.hpp"
#include "UiForms/MainWindow/ModifyExistingModelWindow.hpp"
#include "UiForms/Bot/BotUi.hpp"
#include "UiForms/MainWindow/BotSettingsMainWindow.hpp"

#include <opencv2/dnn.hpp>

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent), settingsManager()
{
    setFixedSize(200, 340);
    setWindowTitle("Main Window");

    settings = settingsManager.loadSettings("assets/settings.json");

    setupUI();
    populateModelsList();
    connectSignals();
}

void MainWindow::setupUI()
{
    clientWindowName = new QLineEdit(this);
    clientWindowName->setFixedSize(150, 40);
    clientWindowName->move(0, 0);
    clientWindowName->setText(settings.windowName.c_str());

    checkHandle = new QPushButton("Check", this);
    checkHandle->setFixedSize(50, 40);
    checkHandle->move(150, 0);

    trainNewModelButton = new QPushButton("Train new model", this);
    trainNewModelButton->setEnabled(false);
    trainNewModelButton->setFixedSize(200, 40);
    trainNewModelButton->move(0, 40);

    loadModelButton = new QPushButton("Load model", this);
    loadModelButton->setEnabled(false);
    loadModelButton->setFixedSize(200, 40);
    loadModelButton->move(0, 80);

    modifyExistingModelButton = new QPushButton("Modify existing model", this);
    modifyExistingModelButton->setEnabled(false);
    modifyExistingModelButton->setFixedSize(200, 40);
    modifyExistingModelButton->move(0, 120);

    trainButton = new QPushButton("Train", this);
    trainButton->setEnabled(false);
    trainButton->setFixedSize(200, 40);
    trainButton->move(0, 160);

    modelsList = new QListWidget(this);
    modelsList->setEnabled(false);
    modelsList->setFixedSize(200, 100);
    modelsList->move(0, 200);

    botOptionsButton = new QPushButton("Bot Options", this);
    botOptionsButton->setEnabled(false);
    botOptionsButton->setFixedSize(100, 40);
    botOptionsButton->move(0, 300);

    startBotButton = new QPushButton("Start Bot", this);
    startBotButton->setEnabled(false);
    startBotButton->setFixedSize(100, 40);
    startBotButton->move(100, 300);
}

void MainWindow::populateModelsList()
{
    QDir modelDir("./assets/models");
    modelDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    const QStringList folders = modelDir.entryList();
    for (const QString& folder : folders)
        modelsList->addItem(folder);
}

void MainWindow::connectSignals()
{
    connect(modelsList,&QListWidget::itemSelectionChanged,
            this, &MainWindow::onModelSelectionChanged);
    connect(checkHandle,&QPushButton::clicked,
            this, &MainWindow::onCheckHandleClicked);
    connect(trainNewModelButton,&QPushButton::clicked,
            this, &MainWindow::onTrainNewModelClicked);
    connect(modifyExistingModelButton, &QPushButton::clicked,
            this, &MainWindow::onModifyExistingModelClicked);
    connect(botOptionsButton,&QPushButton::clicked,
            this, &MainWindow::onBotOptionsClicked);
    connect(startBotButton,&QPushButton::clicked,
            this, &MainWindow::onStartBotClicked);
    connect(trainButton,&QPushButton::clicked,
            this, &MainWindow::onTrainClicked);
    connect(loadModelButton,&QPushButton::clicked,
            this, &MainWindow::onLoadModelClicked);
}

void MainWindow::onModelSelectionChanged()
{
    const bool hasSelection = !modelsList->selectedItems().isEmpty();
    trainButton->setEnabled(hasSelection);
    loadModelButton->setEnabled(hasSelection);
    modifyExistingModelButton->setEnabled(hasSelection);
}

void MainWindow::onCheckHandleClicked()
{
    if (!WindowManager::checkWindowExists(clientWindowName->text().toStdString())) {
        QMessageBox::information(this, "Error", "Window not found!");
        return;
    }

    QMessageBox::information(this, "Info", "Window found successfully!");

    digitsNet = std::make_shared<cv::dnn::Net>(cv::dnn::readNetFromONNX("assets/char_nn.onnx"));
    digitsNet->setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
    digitsNet->setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);

    wm = std::make_shared<WindowManager>(clientWindowName->text().toStdString());

    trainNewModelButton->setEnabled(true);
    modelsList->setEnabled(true);
    botOptionsButton->setEnabled(true);

    settings.windowName = clientWindowName->text().toStdString();
    settingsManager.saveSettings(settings, "assets/settings.json");
}

void MainWindow::onTrainNewModelClicked()
{
    auto* w = new TrainWindow(wm);
    w->show();
}

void MainWindow::onModifyExistingModelClicked()
{
    auto* w = new ModifyExistingModelWindow(wm, modelsList->currentItem()->text());
    w->show();
}

void MainWindow::onBotOptionsClicked()
{
    auto* w = new BotSettingsMainWindow(wm, digitsNet);
    w->show();
}

void MainWindow::onStartBotClicked()
{
    startBotButton->setEnabled(false);

    auto* botUI = new BotUi(wm, mobsNet, digitsNet);
    botUI->show();

    this->hide();
    connect(botUI, &QObject::destroyed, this, [this]() {
        startBotButton->setEnabled(true);
    });
}

void MainWindow::onTrainClicked()
{
    const QString trainingFolder = modelsList->currentItem()->text();
    const QString projectPath = QDir::currentPath() + QString("/assets/models/%1").arg(trainingFolder);

    QString cmd = QString(
        R"(cmd.exe /k "cd /d . && python_venvs\yolotrain\Scripts\activate && cd %1 && yolo detect train model=yolov8s.pt data=dataset.yaml name=out epochs=500 patience=1000 imgsz=640 batch=16 device=0 workers=8 exist_ok=True && yolo export model=./runs/detect/out/weights/best.pt format=onnx opset=12 simplify=True dynamic=False")"
    ).arg(projectPath);

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    CreateProcessA(nullptr, cmd.toStdString().data(),
                   nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE,
                   nullptr, nullptr, &si, &pi);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void MainWindow::onLoadModelClicked()
{
    const QString modelPath = QDir::currentPath() +
        QString("/assets/models/%1/runs/detect/out/weights/best.onnx")
            .arg(modelsList->currentItem()->text());

    if (!QFileInfo::exists(modelPath)) {
        QMessageBox::warning(this, "Error", "Model file not found!");
        return;
    }

    mobsNet = std::make_shared<cv::dnn::Net>(cv::dnn::readNetFromONNX(modelPath.toStdString()));
    mobsNet->setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
    mobsNet->setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);

    startBotButton->setEnabled(true);
    QMessageBox::information(this, "Info", "Model loaded successfully!");
}