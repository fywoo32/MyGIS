/******************************************************************
FileName: MyGraphicsItem.h
Author: Xu Ziyang
Version: 1.3
Date: 2024-06-18
Description:
Function List:

History:
Xuziyang 20/06/24 1.0
Xuziyang 21/06/24 1.1
Xuziyang 25/06/24 1.2
Xuziyang 28/06/24 1.3
*******************************************************************/
#pragma once
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPolygonItem>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsSceneEvent>
#include <QColorDialog>
#include <QPainter>
#include <QVector>
#include <QCursor>
#include <QPointF>

/*============可编辑的面======================================================================================*/
class MyPolygonItem : public QGraphicsPolygonItem {
public:
    MyPolygonItem(const QPolygonF& polygon, QGraphicsItem* parent = nullptr)
        : QGraphicsPolygonItem(polygon, parent) {
        // 设置图形项的属性和行为
        setPen(QPen(Qt::black, 0.002));
        setBrush(QBrush(Qt::lightGray));
        setFlag(QGraphicsItem::ItemIsMovable, true);  // 允许移动
        setFlag(QGraphicsItem::ItemIsSelectable, true);  // 允许选择
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);  // 允许发送几何变化信号
        setAcceptHoverEvents(true);  // 接受鼠标悬停事件

        createHandles(); // 创建悬挂点
    }

protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override {
        for (QGraphicsEllipseItem* handle : mvpHandles) {
            if (handle->contains(event->pos())) {
                setCursor(Qt::SizeAllCursor);  // 如果鼠标在控制点上，则设置为移动光标
                return;
            }
        }
        setCursor(Qt::ArrowCursor);  // 否则设置为箭头光标
        QGraphicsPolygonItem::hoverMoveEvent(event);  // 调用基类的悬停事件处理函数
    }

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
        if (event->button() == Qt::RightButton) {
            QColor color = QColorDialog::getColor(brush().color(), nullptr, "Select Color");
            if (color.isValid()) {
                setBrush(QBrush(color));
            }
            return;
        }
        for (QGraphicsEllipseItem* handle : mvpHandles) {
            if (handle->contains(event->pos())) {
                mpMovingHandle = handle;
                return;
            }
        }
        QGraphicsPolygonItem::mousePressEvent(event);
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override {
        if (mpMovingHandle) {
            QPointF newPos = event->pos();
            mpMovingHandle->setRect(newPos.x() - SHandleRadius, newPos.y() - SHandleRadius, SHandleRadius * 2, SHandleRadius * 2);
            updatePolygonFromHandles();
        }
        else {
            QGraphicsPolygonItem::mouseMoveEvent(event);
        }
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override {
        if (mpMovingHandle) {
            mpMovingHandle = nullptr;
        }
        QGraphicsPolygonItem::mouseReleaseEvent(event);
    }

    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override {
        if (change == QGraphicsItem::ItemPositionChange || change == QGraphicsItem::ItemTransformChange) {
            updateHandles();
        }
        return QGraphicsPolygonItem::itemChange(change, value);
    }

private:
    void createHandles() {
        const QPolygonF& poly = polygon();
        for (const QPointF& point : poly) {
            QGraphicsEllipseItem* handle = new QGraphicsEllipseItem(
                QRectF(point.x() - SHandleRadius, point.y() - SHandleRadius, SHandleRadius * 2, SHandleRadius * 2), this);
            handle->setFlag(QGraphicsItem::ItemIsMovable, true);
            handle->setBrush(Qt::red);
            handle->setPen(QPen(Qt::black, 0.001));
            mvpHandles.append(handle);
        }
        updateHandles();
    }

    void updateHandles() {
        const QPolygonF& poly = polygon();
        for (int i = 0; i < mvpHandles.size(); ++i) {
            mvpHandles[i]->setPos(poly[i]);
        }
    }

    void updatePolygonFromHandles() {
        QPolygonF newPolygon;
        for (QGraphicsEllipseItem* handle : mvpHandles) {
            newPolygon << handle->rect().center();  // 将控制点的中心坐标添加到新的多边形顶点中
        }
        setPolygon(newPolygon);  // 更新多边形的顶点坐标
    }

    static constexpr qreal SHandleRadius = 0.001;
    QVector<QGraphicsEllipseItem*> mvpHandles;
    QGraphicsEllipseItem* mpMovingHandle = nullptr;
};

/*============可编辑的线======================================================================================*/
class MyPathItem : public QGraphicsPathItem {
public:
    MyPathItem(const QPainterPath& path, QGraphicsItem* parent = nullptr)
        : QGraphicsPathItem(path, parent) {
        // 设置图形项的属性和行为
        setPen(QPen(Qt::black, 0.002));
        setBrush(QBrush(Qt::NoBrush));  // 不填充
        setFlag(QGraphicsItem::ItemIsMovable, true);  // 允许移动
        setFlag(QGraphicsItem::ItemIsSelectable, true);  // 允许选择
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);  // 允许发送几何变化信号
        setAcceptHoverEvents(true);  // 接受鼠标悬停事件

        createHandles();  // 创建控制点
    }

protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override {
        for (QGraphicsEllipseItem* handle : mvpHandles) {
            if (handle->contains(event->pos())) {
                setCursor(Qt::SizeAllCursor);  // 如果鼠标在控制点上，则设置为移动光标
                return;
            }
        }
        setCursor(Qt::ArrowCursor);  // 否则设置为箭头光标
        QGraphicsPathItem::hoverMoveEvent(event);  // 调用基类的悬停事件处理函数
    }

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
        if (event->button() == Qt::RightButton) {
            QColor color = QColorDialog::getColor(pen().color(), nullptr, "Select Color");
            if (color.isValid()) {
                setPen(QPen(color, pen().widthF()));
            }
            return;
        }
        for (QGraphicsEllipseItem* handle : mvpHandles) {
            if (handle->contains(event->pos())) {
                mpMovingHandle = handle;
                return;
            }
        }
        QGraphicsPathItem::mousePressEvent(event);
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override {
        if (mpMovingHandle) {
            QPointF newPos = event->pos();
            mpMovingHandle->setRect(newPos.x() - SHandleRadius, newPos.y() - SHandleRadius, SHandleRadius * 2, SHandleRadius * 2);
            updatePathFromHandles();
        }
        else {
            QGraphicsPathItem::mouseMoveEvent(event);
        }
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override {
        if (mpMovingHandle) {
            mpMovingHandle = nullptr;
        }
        QGraphicsPathItem::mouseReleaseEvent(event);
    }

    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override {
        if (change == QGraphicsItem::ItemPositionChange || change == QGraphicsItem::ItemTransformChange) {
            updateHandles();
        }
        return QGraphicsPathItem::itemChange(change, value);
    }

private:
    void createHandles() {
        const QPainterPath& path = this->path();
        for (int i = 0; i < path.elementCount(); ++i) {
            const QPainterPath::Element& element = path.elementAt(i);
            QGraphicsEllipseItem* handle = new QGraphicsEllipseItem(
                QRectF(element.x - SHandleRadius, element.y - SHandleRadius, SHandleRadius * 2, SHandleRadius * 2), this);
            handle->setFlag(QGraphicsItem::ItemIsMovable, true);
            handle->setBrush(Qt::red);
            handle->setPen(QPen(Qt::black, 0.001));
            mvpHandles.append(handle);
        }
        updateHandles();
    }

    void updateHandles() {
        const QPainterPath& path = this->path();
        for (int i = 0; i < mvpHandles.size(); ++i) {
            const QPainterPath::Element& element = path.elementAt(i);
            mvpHandles[i]->setPos(QPointF(element.x, element.y));
        }
    }

    void updatePathFromHandles() {
        QPainterPath newPath;
        newPath.moveTo(mvpHandles[0]->rect().center());
        for (int i = 1; i < mvpHandles.size(); ++i) {
            newPath.lineTo(mvpHandles[i]->rect().center());
        }
        setPath(newPath);
    }

    static constexpr qreal SHandleRadius = 0.001;
    QVector<QGraphicsEllipseItem*> mvpHandles;
    QGraphicsEllipseItem* mpMovingHandle = nullptr;
};

/*============可编辑的点======================================================================================*/
class MyPointItem : public QGraphicsEllipseItem {
public:
    MyPointItem(const QPointF& point, QGraphicsItem* parent = nullptr)
        : QGraphicsEllipseItem(QRectF(point.x() - SHandleRadius, point.y() - SHandleRadius, SHandleRadius * 2, SHandleRadius * 2), parent) {
        setBrush(Qt::blue);
        setPen(QPen(Qt::black, 0.002));
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setFlag(QGraphicsItem::ItemIsSelectable, true);
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
        if (event->button() == Qt::RightButton) {
            QColor color = QColorDialog::getColor(brush().color(), nullptr, "Select Color");
            if (color.isValid()) {
                setBrush(QBrush(color));
            }
            return;
        }
        QGraphicsEllipseItem::mousePressEvent(event);
    }

private:
    static constexpr qreal SHandleRadius = 0.001;
};
