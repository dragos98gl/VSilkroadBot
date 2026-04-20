#include "CustomUiObj/RectImageView.hpp"

RectImageView::RectImageView(QWidget* parent) : QLabel(parent) {
    setMouseTracking(true); 
}

void RectImageView::setImage(const cv::Mat& img) {
    cv::Mat matRGB;
    cv::cvtColor(img, matRGB, cv::COLOR_BGR2RGB);

    QImage imgQ(matRGB.data, matRGB.cols, matRGB.rows,
                static_cast<int>(matRGB.step), QImage::Format_RGB888);
    currentImage = img.clone();
     
    pixmapOriginal = QPixmap::fromImage(imgQ.copy());  

    setPixmap(pixmapOriginal);
    update();
}

void RectImageView::setRectangles(const QVector<QRect>& rects)
{
    rectangles = rects;
    update();
    emit rectListUpdated(rectangles);
}

void RectImageView::setImageAndRects(const cv::Mat& img, const QVector<QRect>& rects)
{
    setImage(img);
    rectangles = rects;
    update();
    emit rectListUpdated(rectangles);
}

QVector<QRect> RectImageView::getRectangles() const {
    return rectangles;
}

cv::Mat RectImageView::getCurrentImage() const {
    return currentImage;
}

void RectImageView::removeLastRectangle() {
    if (!rectangles.isEmpty()) {
        rectangles.removeLast();
        update();
        emit rectListUpdated(rectangles);
    }
}

void RectImageView::removeAllRectangles() {
    rectangles.clear();
    update();
    emit rectListUpdated(rectangles);
}

void RectImageView::removeRectangle(int index)
{
    if (index < 0 || index >= rectangles.size())
        return;
    
    rectangles.remove(index);
    update();
    emit rectListUpdated(rectangles);
}

void RectImageView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        drawing = true;
        rectStart = event->pos();
        rectCurrent = rectStart;
    }
}

void RectImageView::mouseMoveEvent(QMouseEvent* event) {
    if (drawing) {
        rectCurrent = event->pos();
        update();
    }
}

void RectImageView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && drawing) {
        drawing = false;
        rectCurrent = event->pos();
        rectangles.append(QRect(rectStart, rectCurrent).normalized());
        emit rectListUpdated(rectangles);
        update();
    }
}

void RectImageView::paintEvent(QPaintEvent* event) {
    QLabel::paintEvent(event);
    if (!pixmap() || pixmap().isNull()) return;

    QPainter painter(this);

    // 1. draw all rectangles
    for (int i = 0; i < rectangles.size(); ++i) {
            if (i == selectedRectIndex)
            painter.setPen(QPen(Qt::green, 2)); // green highlight
        else
            painter.setPen(QPen(Qt::red, 2));

        painter.drawRect(rectangles[i]);
    }

    // 2. draw the rectangle currently being drawn
    if (drawing) {
        painter.setPen(QPen(Qt::red, 2));
        painter.drawRect(QRect(rectStart, rectCurrent).normalized());
    }
}

void RectImageView::setSelectedRect(int index)
{
    if (index < -1 || index >= rectangles.size())
        return;

    selectedRectIndex = index;
    update();
}

int RectImageView::getSelectedRect() const
{
    return selectedRectIndex;
}