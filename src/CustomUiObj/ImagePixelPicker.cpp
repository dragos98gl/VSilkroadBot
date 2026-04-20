
#include "CustomUiObj/ImagePixelPicker.hpp"

ImagePixelPicker::ImagePixelPicker(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    setupLayout();
    setupConnections();
}

void ImagePixelPicker::setupUi()
{
    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    imageLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void ImagePixelPicker::setupLayout()
{
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(imageLabel);
    setLayout(layout);
}

void ImagePixelPicker::setupConnections()
{
}

void ImagePixelPicker::setImage(const cv::Mat img)
{
    if (img.empty())
        return;

    image = QImage(
        img.data,
        img.cols,
        img.rows,
        static_cast<int>(img.step),
        QImage::Format_ARGB32   
    ).copy(); 

    imageLabel->setPixmap(QPixmap::fromImage(image));
    imageLabel->setFixedSize(image.size());
    setFixedSize(image.size());
}

void ImagePixelPicker::mousePressEvent(QMouseEvent* event)
{
    if (image.isNull())
        return;

    QPoint pos = imageLabel->mapFrom(this, event->pos());

    if (!image.rect().contains(pos))
        return;

    QColor color = image.pixelColor(pos);
    emit pixelClicked(PixelInfo{ pos, color });
}