#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QColor>

#include "IO/WindowManager.hpp"
#include "UiForms/PixelPickerDialog.hpp"
#include "interfaces.hpp"

class SettingsWindowRgbPosWidget : public QWidget, public ICustomView
{
    Q_OBJECT
public:
    explicit SettingsWindowRgbPosWidget(const QString& title,
                                std::shared_ptr<WindowManager> wm,
                                PixelInfo currnentValue,
                                QWidget* parent = nullptr);
    PixelInfo getResult() const;
private:
    void updateColor(const QColor& color);
    void onEditClicked();

private:
    void setupUi() override;
    void setupLayout() override;
    void setupConnections() override;

    std::shared_ptr<WindowManager> wm;
    const QString& title;

    PixelInfo currentValue;

    QVBoxLayout* textLayout;
    QHBoxLayout* rightLayout;
    QHBoxLayout* mainLayout;

    QLabel* titleLabel;
    QLabel* rgbLabel;
    QLabel* posLabel;
    QWidget* colorDisplay;
    QPushButton* editButton;
};
