#pragma once

#include <QTreeView>
#include <QMenu>
#include <QStandardItemModel>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QColorDialog>
#include <QPainter>
#include <QBrush>

#include "MyItem.h"

class MyLayerManager;
class MyTreeView : public QTreeView {
    Q_OBJECT

public:
    MyTreeView(QWidget* parent = nullptr);
    ~MyTreeView();
    
    void connectScene(QGraphicsScene* pScene) {
        mpScene = pScene;
    }

    void connectLayerManager(MyLayerManager* pManager) {
        mpManager = pManager;
    }

private slots:
    void openContextMenu(const QPoint& position);
    void onItemChanged(QStandardItem* item);
    void performAction1();
    void performAction2();
    void performAction3();
    void performAction4();
    void performAction5();

private:
    MyItem* mpChoosedItem;
    QStandardItemModel* mpModel;
    QGraphicsScene* mpScene;
    MyLayerManager* mpManager;
};

