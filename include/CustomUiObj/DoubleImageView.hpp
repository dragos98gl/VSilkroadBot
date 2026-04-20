#pragma once

#include <QWidget>
#include <QPainter>
#include <QMutex>
#include <QImage>

class DoubleImageView : public QWidget
{
    Q_OBJECT
public:
    explicit DoubleImageView(QWidget* parent = nullptr);
    void setTopImage(const QImage& img);
    void setBottomImage(const QImage& img);

protected:
    void paintEvent(QPaintEvent*) override;
private:
    QImage topImage;
    QImage bottomImage;
    QMutex mutex;
};