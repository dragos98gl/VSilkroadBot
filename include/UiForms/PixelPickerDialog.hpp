#pragma once

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QColor>
#include <QPalette>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "interfaces.hpp"
#include "CustomUiObj/ImagePixelPicker.hpp"
#include "IO/WindowManager.hpp"

class PixelPickerDialog : public QDialog, public ICustomView{
    Q_OBJECT
public:
    explicit PixelPickerDialog(std::shared_ptr<WindowManager> wm, QWidget* parent = nullptr);
    PixelInfo getResult() const;

private:
    void setupUi() override;
    void setupLayout() override;
    void setupConnections() override;
    void onPixelClicked(const PixelInfo& info);
    void onCapture();
    void onSave();

    std::shared_ptr<WindowManager> wm;
    ImagePixelPicker* imagePixelPicker;
    QPushButton* captureButton;
    QPushButton* saveButton;
    QLabel* RGBLabel;
    QLabel* PositionLabel;
    QWidget* colorDisplay;

    PixelInfo currentResult;
};
