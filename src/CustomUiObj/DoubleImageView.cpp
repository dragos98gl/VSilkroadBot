#include "CustomUiObj/DoubleImageView.hpp"

DoubleImageView::DoubleImageView(QWidget* parent)
    : QWidget(parent)
{
    setFixedSize(320, 500);
}

void DoubleImageView::setTopImage(const QImage& img)
{
    QMutexLocker locker(&mutex);
    topImage = img;
    update();
}

void DoubleImageView::setBottomImage(const QImage& img)
{
    QMutexLocker locker(&mutex);
    bottomImage = img;
    update();
}

void DoubleImageView::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    QMutexLocker locker(&mutex);

    if (!topImage.isNull())
        painter.drawImage(0, 0, topImage);

    if (!bottomImage.isNull())
        painter.drawImage(0, 320, bottomImage);
}