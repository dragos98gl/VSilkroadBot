#pragma once

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QHBoxLayout>

#include "IO/WindowManager.hpp"
#include "CustomUiObj/RectImageView.hpp"
#include "Bot/FrameProcess.hpp"
#include "interfaces.hpp"

class ModifyExistingModelWindow : public QWidget, public ICustomView
{
    Q_OBJECT
public:
    ModifyExistingModelWindow(std::shared_ptr<WindowManager> wm, QString modelName, QWidget* parent = nullptr);

private slots:
    void onCapture();
    void onRemove();
    void onRectsUpdated(const QVector<QRect>& rects);
    void onFileSelected(int row);
    void onSave();
    void onRectListItemSelected(int row);

private:
    void setupUi() override;
    void setupLayout() override;
    void setupConnections() override;

    void saveYaml(const QString& recordName);
    std::vector<QRect> qVectorToStdVector(const QVector<QRect>& v);

    std::shared_ptr<WindowManager> wm;
    QString modelName;

    // UI widgets
    RectImageView* view = nullptr;
    QPushButton* captureButton = nullptr;
    QPushButton* removeRecordButton = nullptr;
    QPushButton* saveRecordButton = nullptr;
    QLabel* saveRecordLabel = nullptr;
    QButtonGroup* trainTypeGroup = nullptr;
    QRadioButton* trainButton = nullptr;
    QRadioButton* valButton = nullptr;
    QListWidget* filesList = nullptr;
    QListWidget* rectList = nullptr;
    QCheckBox* assistedModeCheck = nullptr;

    // Network (optional)
    std::unique_ptr<cv::dnn::Net> mobsNet;

    // --- New: keep all loaded models in memory ---
    std::vector<TrainingModel> trainingModels;
};