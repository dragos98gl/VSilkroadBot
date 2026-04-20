#pragma once

#include <QWidget>
#include <QImage>
#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QDebug>

#include <opencv2/opencv.hpp>

#include "types.hpp"
#include "interfaces.hpp"

class ImagePixelPicker : public QWidget, public ICustomView
{
    Q_OBJECT
public:
    explicit ImagePixelPicker(QWidget* parent = nullptr);
    void setImage(const cv::Mat img);

    void setupUi() override;
    void setupLayout() override;
    void setupConnections() override;

signals:
    void pixelClicked(const PixelInfo& info);

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    QLabel* imageLabel;
    QVBoxLayout* layout;
    QImage image;
};
