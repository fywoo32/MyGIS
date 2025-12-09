/******************************************************************
FileName: MyGraphicsView.h
Author: Xu Ziyang
Version: 1.3
Date: 2024-06-18
Description:
Function List:
1. MyGraphicsView
2. wheelEvent
3. mousePressEvent
4. mouseMoveEvent
5. mouseReleaseEvent
History:
Xuziyang 20/06/24 1.0
Xuziyang 21/06/24 1.1
Xuziyang 25/06/24 1.2
Xuziyang 28/06/24 1.3
*******************************************************************/
#pragma once
#include <QGraphicsView>
#include <QGraphicsPathItem>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <iostream>

class MyGraphicsView : public QGraphicsView {
    Q_OBJECT
public:
    MyGraphicsView(QWidget* parent) : QGraphicsView(parent), _isPanning(true), _panStartX(0), _panStartY(0) {
        setRenderHint(QPainter::Antialiasing);
        setDragMode(QGraphicsView::NoDrag);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setMouseTracking(true);//开启鼠标追踪
        //以鼠标为中心
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);//设置变换锚点
        setResizeAnchor(QGraphicsView::AnchorUnderMouse);//设置缩放锚点
        setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    }
    void setEditing(bool isEditing) {
        _isEditing = isEditing;
    }
    double orignalsize = 1.0;

protected:
    void wheelEvent(QWheelEvent* event) override {
        const double zoomInFactor = 1.25;
        const double zoomOutFactor = 1.0 / zoomInFactor;

        QPointF oldPos = mapToScene(event->position().toPoint());

        double zoomFactor = (event->angleDelta().y() > 0) ? zoomInFactor : zoomOutFactor;
        scale(zoomFactor, zoomFactor);

        QPointF newPos = mapToScene(event->position().toPoint());

        QPointF delta = newPos - oldPos;
        translate(delta.x(), delta.y());

        int numDegrees = event->angleDelta().y() / 8;
        int numSteps = numDegrees / 15;
        qreal factor = std::pow(1.2, numSteps);
        scaleView(factor);
    }
    void mousePressEvent(QMouseEvent* event) override {
        if (_isEditing) {
            QGraphicsView::mousePressEvent(event);
            return;
        }
        if (event->button() == Qt::LeftButton) {
            _isPanning = true;
            setCursor(Qt::ClosedHandCursor);
            _panStartX = event->x();
            _panStartY = event->y();
        }
        QGraphicsView::mousePressEvent(event);
    }
    void mouseMoveEvent(QMouseEvent* event) override {
        if (_isEditing) {
            QGraphicsView::mouseMoveEvent(event);
            return;
        }
        if (_isPanning) {
            int deltaX = event->x() - _panStartX;
            int deltaY = event->y() - _panStartY;
            _panStartX = event->x();
            _panStartY = event->y();
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() - deltaX);
            verticalScrollBar()->setValue(verticalScrollBar()->value() - deltaY);
        }
        QGraphicsView::mouseMoveEvent(event);
    }
    void mouseReleaseEvent(QMouseEvent* event) override {
        if (_isEditing) {
            QGraphicsView::mouseReleaseEvent(event);
            return;
        }
        if (event->button() == Qt::LeftButton) {
            _isPanning = false;
            setCursor(Qt::ArrowCursor);
        }
        QGraphicsView::mouseReleaseEvent(event);
    }
    void scaleView(qreal scaleFactor) {
        qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
        if (factor < 0.07 || factor > 100)
            return;

        scale(scaleFactor, scaleFactor);

        // 动态调整线条和点的粗细
        QList<QGraphicsItem*> items = scene()->items();
        for (QGraphicsItem* item : items) {
            if (QGraphicsPathItem* pathItem = dynamic_cast<QGraphicsPathItem*>(item)) {
                QPen pen = pathItem->pen();
                pen.setWidthF(1.0 / factor); // 根据缩放因子调整线条宽度
                pathItem->setPen(pen);
            }
            else if (QGraphicsEllipseItem* ellipseItem = dynamic_cast<QGraphicsEllipseItem*>(item)) {
                QPen pen = ellipseItem->pen();
                pen.setWidthF(1.0 / factor); // 根据缩放因子调整点的边框宽度
                ellipseItem->setPen(pen);

                // 动态调整点的尺寸
                QRectF rect = ellipseItem->rect();
                double centerX = rect.center().x();
                double centerY = rect.center().y();
                double newSize = orignalsize / factor; // originalSize 是你定义的初始点大小
                orignalsize = newSize;

                // 更新点的大小，同时保持中心点不变
                ellipseItem->setRect(centerX - newSize / 2, centerY - newSize / 2, newSize, newSize);
            }
        }
    }

private:
    bool _isPanning;
    bool _isEditing;
    int _panStartX;
    int _panStartY;
};

