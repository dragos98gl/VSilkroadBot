#pragma once

#include <QApplication>
#include <QLabel>
#include <QPixmap>
#include <QMouseEvent>
#include <QPainter>
#include <QRect>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

class RectImageView : public QLabel {
    Q_OBJECT
public:
    RectImageView(QWidget* parent = nullptr);
    void setImage(const cv::Mat& img);
    QVector<QRect> getRectangles() const;
    void removeLastRectangle();
    void removeRectangle(int index);
    void removeAllRectangles();
    cv::Mat getCurrentImage() const;
    void setRectangles(const QVector<QRect>& rects);
    void setImageAndRects(const cv::Mat& img, const QVector<QRect>& rects);
    void setSelectedRect(int index);
    int getSelectedRect() const; 

signals:
    void rectListUpdated(const QVector<QRect>& rect);
    void save();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    QPixmap pixmapOriginal;
    bool drawing = false;
    QPoint rectStart;
    QPoint rectCurrent;
    QVector<QRect> rectangles;
    cv::Mat currentImage;
    
    int selectedRectIndex = -1;
};