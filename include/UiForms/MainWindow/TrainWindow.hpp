#pragma once

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>

#include <QRadioButton>
#include <QButtonGroup>
#include <QHBoxLayout>

#include "IO/WindowManager.hpp"
#include "CustomUiObj/RectImageView.hpp"
#include "interfaces.hpp"

class TrainWindow : public QWidget, public ICustomView {
    Q_OBJECT
public:
    explicit TrainWindow(std::shared_ptr<WindowManager> wm, QWidget* parent = nullptr);
private slots:
    void onCapture();
    void onRemove();
    void onRectsUpdated(const QVector<QRect>& rects);
    void onSave();
    void onTextChanged(const QString& text);

private:
    void setupUi() override;
    void setupLayout() override;
    void setupConnections() override;

    void saveYaml(const QString& recordName);

    std::shared_ptr<WindowManager> wm;

    RectImageView* view;
    QPushButton* captureButton;
    QPushButton* removeRecordButton;
    QPushButton* saveRecordButton;
    QListWidget* rectList;
    QLabel* saveRecordLabel;
    QLineEdit* saveRecordName;
    QLabel* valCountLabel;
    QLabel* trainCountLabel;

    QButtonGroup* trainTypeGroup;
    QRadioButton* valButton;
    QRadioButton* trainButton;

    uint8_t valCount=0;
    uint8_t trainCount=0;
};
