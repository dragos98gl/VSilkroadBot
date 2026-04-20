#pragma once

#include <QWidget>
#include <QImage>
#include <QPoint>
#include <QRect>
#include <vector>

#include "types.hpp"

class BotNavigationMapView : public QWidget {
    Q_OBJECT
public:
    explicit BotNavigationMapView(QWidget* parent = nullptr);

    // load image from path; returns true if successful
    bool loadImage(const QString& path, QPoint initPos);

    void setCircleRadius(int r) { circleSettigs.nElement.radius = r;}

    void syncCircles(const NavigationStructure* zone);
signals:
    void circleListChanged(const MapCircle circle);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override; // optional: zoom
    void leaveEvent(QEvent* event) override;

private:
    void ensureViewportInBounds();

    const double minZoom = 0.25;
    const double maxZoom = 5.0;
    const double zoomStep = 1.15f;

    QImage image;
    QPoint viewTopLeft;      // (x,y) pixel coordinates of the top-left corner of the viewport in the image
    MapCircle circleSettigs;
    std::vector<MapCircle> circles; // positions in image coordinates

    // mouse state to distinguish click from drag
    bool mousePressed = false;
    QPoint pressPosWidget;
    QPoint lastMousePosWidget;
    bool dragging = false;
    const int dragThreshold = 4; // pixel widget

    QPointF hoverPos;       // mouse position on the image
    bool hoverValid = false; // whether the mouse is over the image

    double zoom = 1.0; // 1.0 => 1:1 (in case you want to add zoom)
};
