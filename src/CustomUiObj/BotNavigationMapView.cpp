#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

#include "CustomUiObj/BotNavigationMapView.hpp"
BotNavigationMapView::BotNavigationMapView(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    viewTopLeft = QPoint(0,0);
}

bool BotNavigationMapView::loadImage(const QString& path, QPoint initPos)
{
    QImage img;
    if (!img.load(path)) {
        return false;
    }
    image = img.convertToFormat(QImage::Format_ARGB32); // format that supports alpha
    viewTopLeft = initPos;
    ensureViewportInBounds();
    update();
    return true;
}

void BotNavigationMapView::ensureViewportInBounds()
{
    if (image.isNull()) return;

    int viewW = int(width() / zoom);
    int viewH = int(height() / zoom);

    int maxX = qMax(0, image.width() - viewW);
    int maxY = qMax(0, image.height() - viewH);

    viewTopLeft.setX(qBound(0, viewTopLeft.x(), maxX));
    viewTopLeft.setY(qBound(0, viewTopLeft.y(), maxY));
}

void BotNavigationMapView::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::black);

    if (image.isNull()) return;

    p.setRenderHint(QPainter::Antialiasing, true);

    // Draw the image (same as before)
    int viewW = int(width() / zoom);
    int viewH = int(height() / zoom);
    QRect src(viewTopLeft, QSize(viewW, viewH));
    QRect imgRect(QPoint(0,0), image.size());
    src = src.intersected(imgRect);
    QRect dst(0, 0, int(src.width()*zoom), int(src.height()*zoom));
    p.drawImage(dst, image, src);

    // Draw existing circles
    for (const MapCircle& mCircle : circles) 
    {
        p.setBrush(mCircle.color);
        p.setPen(Qt::NoPen);
        QPointF widgetPos = (QPointF(mCircle.nElement.pos.first,mCircle.nElement.pos.second) - viewTopLeft) * zoom;
        p.drawEllipse(widgetPos, mCircle.nElement.radius*zoom, mCircle.nElement.radius*zoom);
    }

    // 🔹 Cerc preview (hover)
    if (hoverValid) 
    {
        p.setBrush(QColor(0, 255, 0, 80)); // semi-transparent green
        p.setPen(Qt::NoPen);
        QPointF widgetPos = (hoverPos - viewTopLeft) * zoom;
        p.drawEllipse(widgetPos, circleSettigs.nElement.radius*zoom, circleSettigs.nElement.radius*zoom);
    }
}

void BotNavigationMapView::wheelEvent(QWheelEvent* event)
{
    if (image.isNull()) return;

    double oldZoom = zoom;

    if (event->angleDelta().y() > 0)
        zoom *= zoomStep;
    else
        zoom /= zoomStep;

    zoom = qBound(minZoom, zoom, maxZoom);

    if (qFuzzyCompare(oldZoom, zoom))
        return;

    // mouse position in image coordinates before zoom
    QPointF mouseImgPos =
        viewTopLeft + (event->position() / oldZoom);

    // recalculate viewTopLeft so that the point under the mouse stays fixed
    viewTopLeft = (mouseImgPos - (event->position() / zoom)).toPoint();

    ensureViewportInBounds();
    update();

    event->accept();
}

void BotNavigationMapView::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    ensureViewportInBounds();
    update();
}

void BotNavigationMapView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) 
    {
        if (image.isNull()) return;

        mousePressed = true;
        dragging = false;
        pressPosWidget = event->pos();
        lastMousePosWidget = event->pos();
    }
}

void BotNavigationMapView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) 
    {
        if (image.isNull()) return;

        
        if (!dragging) 
        {
            QPointF widgetPos = event->position();      // widget coordinates
            QPointF imgPosF = viewTopLeft + (widgetPos / zoom);
            QPoint imagePos(
                qBound(0, int(imgPosF.x()), image.width()  - 1),
                qBound(0, int(imgPosF.y()), image.height() - 1)
            );
            emit circleListChanged({{{imagePos.x(),imagePos.y()},circleSettigs.nElement.type,circleSettigs.nElement.radius},circleSettigs.color});
        }

        mousePressed = false;
        dragging = false;
    } 
}

void BotNavigationMapView::syncCircles(const NavigationStructure* zone) 
{
    circles.clear(); 
    if (!zone) return; 
    auto addElems = [&](const std::vector<NavigationElement>& elems, NavigationType type, const QColor& color) 
    { 
        for (const auto& e : elems) 
        { 
            MapCircle c; 
            c.nElement = e; 
            c.nElement.type = type; 
            c.color = color; 
            circles.push_back(c); 
        } 
    }; 
    
    addElems(zone->attackingArea, NavigationType::ATTACKING_AREA, QColor(0,0,255,80)); 
    addElems(zone->toGoWhenNoMobs, NavigationType::TO_GO_WHEN_NO_MOBS, QColor(0,255,0,80)); 
    addElems(zone->trajectory, NavigationType::TRAJECTORY, QColor(255,0,0,200)); 
    addElems(zone->bypass, NavigationType::BYPASS, QColor(255,255,255,80)); 
    
    update(); 
}

void BotNavigationMapView::mouseMoveEvent(QMouseEvent* event)
{
    if (image.isNull()) return;

    QPointF imgPos = viewTopLeft + (event->position() / zoom);

    if (imgPos.x() >= 0 && imgPos.x() < image.width() &&
        imgPos.y() >= 0 && imgPos.y() < image.height()) {
        hoverPos = imgPos;
        hoverValid = true;
    } else {
        hoverValid = false;
    }

    if (!mousePressed) {
        update(); // redraw for preview
        return;
    }

    QPoint delta = event->pos() - pressPosWidget;
    if (!dragging && (delta.manhattanLength() >= dragThreshold)) {
        dragging = true;
    }

    if (dragging) {
        QPoint deltaMove = event->pos() - lastMousePosWidget;
        viewTopLeft -= deltaMove;
        ensureViewportInBounds();
        lastMousePosWidget = event->pos();
        update();
    }
}

void BotNavigationMapView::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);
    hoverValid = false; // mouse left the widget
    update();
}