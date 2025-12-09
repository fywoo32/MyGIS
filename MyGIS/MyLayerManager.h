#pragma once

#include <QTreeWidget>
#include <QGraphicsView>
#include <QStandardItemModel>
#include <QGraphicsScene>
#include <QCheckBox>
#include <QIcon>

#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "MyGraphicsView.h"
#include "MyTreeView.h"

class MyLayerManager {
public:
    MyLayerManager(MyTreeView* pTreeView, MyGraphicsView* pGraphicsView, QAction* pEdit, QAction* pAttributes)
        : mpTreeView(pTreeView), mpGraphicsView(pGraphicsView), mpEdit(pEdit), mpAttributes(pAttributes) {
        pTreeView->connectLayerManager(this);
        pTreeView->connectScene(pGraphicsView->scene());
    }

    ~MyLayerManager() {}

    QStringList mlVectorLayers;      // 矢量图层路径列表
    QStringList mlRasterLayers;      // 栅格图层路径列表
    //QList<MyVectorLayer*> mlpVectorLayer;
    //QList<MyRasterLayer*> mlpRasterLayer;

    void addVectorLayer(MyVectorLayer* pVectorLayer) {
        //mlpVectorLayer.append(pVectorLayer);
        MyItem* pNewItem = new MyItem(pVectorLayer, pVectorLayer->getName());
        pNewItem->setCheckable(true);
        pNewItem->setCheckState(pVectorLayer->isVisible() ? Qt::Checked : Qt::Unchecked);
        if (pNewItem->getVectorLayer()->getType() == GeometryType::Polygon) {
            QIcon icon(":/MyGIS/icons/polygon.png");
            pNewItem->setIcon(icon);
        }
        else if (pNewItem->getVectorLayer()->getType() == GeometryType::Line) {
            QIcon icon(":/MyGIS/icons/line.png");
            pNewItem->setIcon(icon);
        }
        else if (pNewItem->getVectorLayer()->getType() == GeometryType::Point) {
            QIcon icon(":/MyGIS/icons/point.png");
            pNewItem->setIcon(icon);
        }
        qobject_cast<QStandardItemModel*>(mpTreeView->model())->appendRow(pNewItem);
    }

    void addRasterLayer(MyRasterLayer* pRasterLayer) {
        //mlpRasterLayer.append(pRasterLayer);
        MyItem* pNewItem = new MyItem(pRasterLayer, pRasterLayer->getName());
        pNewItem->setCheckable(true);
        pNewItem->setCheckState(pRasterLayer->isVisible() ? Qt::Checked : Qt::Unchecked);
        if (pNewItem->getRasterLayer()) {
            QIcon icon(":/MyGIS/icons/raster.png");
            pNewItem->setIcon(icon);
        }
        qobject_cast<QStandardItemModel*>(mpTreeView->model())->appendRow(pNewItem);
    }

    void deleteVectorLayer(MyVectorLayer* pVectorLayer) {
        //mlpVectorLayer.removeOne(pVectorLayer);
        mlVectorLayers.removeOne(pVectorLayer->getFilePath());
        // 删除场景中的图形
        for (auto graphicsItem : pVectorLayer->getGraphicsItem()) {
            if (graphicsItem) {
                mpGraphicsView->scene()->removeItem(graphicsItem);
            }
        }
        mpEdit->setEnabled(false);
        mpAttributes->setEnabled(false);
        // 删除图层对象
        delete pVectorLayer;
        pVectorLayer = nullptr;
    }

    void deleteRasterLayer(MyRasterLayer* pRasterLayer) {
        //mlpRasterLayer.removeOne(pRasterLayer);
        mlRasterLayers.removeOne(pRasterLayer->getFilePath());
        QGraphicsPixmapItem* pPixmapItem = pRasterLayer->getRasterData();
        if (pPixmapItem) {
            mpGraphicsView->scene()->removeItem(pPixmapItem);
        }
        mpEdit->setEnabled(false);
        mpAttributes->setEnabled(false);
        // 删除图层对象
        delete pRasterLayer;
        pRasterLayer = nullptr;
    }

    // 缩放至图层
    void moveToVectorLayer(MyVectorLayer* pVectorLayer) {
        QRectF boundingRect = pVectorLayer->getGraphicsItem()[0]->boundingRect();
        for (const auto& item : pVectorLayer->getGraphicsItem()) {
            boundingRect = boundingRect.united(item->boundingRect());
        }
        mpGraphicsView->fitInView(boundingRect, Qt::KeepAspectRatio);
    }

    void moveToRasterLayer(MyRasterLayer* pRasterLayer) {
        double originX = pRasterLayer->getOriginX();  // 左上角X
        double originY = pRasterLayer->getOriginY();  // 左上角Y
        double pixelWidth = pRasterLayer->getPixelWidth();  // 每个像素的宽度
        double pixelHeight = pRasterLayer->getPixelHeight();  // 每个像素的高
        // 获取地理范围
        QRectF boundingRect(QPointF(originX, originY + pRasterLayer->getColCount() * pixelHeight),
            QSizeF(pRasterLayer->getRowCount() * pixelWidth, pRasterLayer->getColCount() * std::abs(pixelHeight)));
        // 视窗调整
        mpGraphicsView->fitInView(boundingRect, Qt::KeepAspectRatio);
    }

    // 图层上移
    void upVectorLayer(MyVectorLayer* pVectorLayer) {
        for (auto graphicsItem : pVectorLayer->getGraphicsItem()) {
            qreal zValue = graphicsItem->zValue();
            graphicsItem->setZValue(zValue + 1);
        }
        pVectorLayer->setZValue(pVectorLayer->getGraphicsItem()[0]->zValue());
    }

    void upRasterLayer(MyRasterLayer* pRasterLayer) {
        QGraphicsPixmapItem* pPixmapItem = pRasterLayer->getRasterData();
        qreal zValue = pPixmapItem->zValue();
        pPixmapItem->setZValue(zValue + 1);
        pRasterLayer->setZValue(zValue);
    }

    // 图层下移
    void downVectorLayer(MyVectorLayer* pVectorLayer) {
        for (auto graphicsItem : pVectorLayer->getGraphicsItem()) {
            qreal zValue = graphicsItem->zValue();
            graphicsItem->setZValue(zValue - 1);
        }
        pVectorLayer->setZValue(pVectorLayer->getGraphicsItem()[0]->zValue());
    }

    void downRasterLayer(MyRasterLayer* pRasterLayer) {
        QGraphicsPixmapItem* pPixmapItem = pRasterLayer->getRasterData();
        qreal zValue = pPixmapItem->zValue();
        pPixmapItem->setZValue(zValue - 1);
        pRasterLayer->setZValue(zValue);
    }

private:
    MyTreeView* mpTreeView;
    MyGraphicsView* mpGraphicsView;
    QAction* mpEdit;
    QAction* mpAttributes;
};
