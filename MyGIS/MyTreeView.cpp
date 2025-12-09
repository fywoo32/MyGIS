#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#include "MyTreeView.h"
#include "MyLayerManager.h"

MyTreeView::MyTreeView(QWidget* parent)
    : QTreeView(parent), mpChoosedItem(nullptr) {
    mpModel = new QStandardItemModel(this);
    setModel(mpModel);
    setContextMenuPolicy(Qt::CustomContextMenu);
    
    connect(this, &QTreeView::customContextMenuRequested, this, &MyTreeView::openContextMenu);
    connect(mpModel, &QStandardItemModel::itemChanged, this, &MyTreeView::onItemChanged);
}

MyTreeView::~MyTreeView() {
    delete mpModel;
}

void MyTreeView::openContextMenu(const QPoint& position) {
    QModelIndex index = indexAt(position);
    if (index.isValid()) {
        mpChoosedItem = static_cast<MyItem*>(mpModel->itemFromIndex(index));

        if (mpChoosedItem) {
            QMenu menu;
            QAction* action1 = menu.addAction(QIcon(":/MyGIS/icons/up.png"),"向上");
            QAction* action2 = menu.addAction(QIcon(":/MyGIS/icons/down.png"), "向下");
            QAction* action3 = menu.addAction(QIcon(":/MyGIS/icons/delete.png"), "删除");
            menu.addSeparator();
            QAction* action4 = menu.addAction(QIcon(":/MyGIS/icons/zoom.png"), "缩放至图层");
            QAction* action5 = menu.addAction(QIcon(":/MyGIS/icons/color.png"), "设置图层颜色");

            if (mpChoosedItem->getVectorLayer()) {
                if (mpChoosedItem->getVectorLayer()->isEditing()) {
                    action1->setEnabled(false);
                    action2->setEnabled(false);
                    action3->setEnabled(false);
                    action4->setEnabled(false);
                    action5->setEnabled(false);
                }
            }

            connect(action1, &QAction::triggered, this, &MyTreeView::performAction1);
            connect(action2, &QAction::triggered, this, &MyTreeView::performAction2);
            connect(action3, &QAction::triggered, this, &MyTreeView::performAction3);
            connect(action4, &QAction::triggered, this, &MyTreeView::performAction4);
            connect(action5, &QAction::triggered, this, &MyTreeView::performAction5);

            menu.exec(viewport()->mapToGlobal(position));
        }
    }
}

void MyTreeView::onItemChanged(QStandardItem* item) {
    MyItem* pItem = static_cast<MyItem*>(item);
    if (pItem->getVectorLayer()) {
        if (pItem->checkState() == Qt::CheckState::Unchecked) {
            for (auto graphicsItem : pItem->getVectorLayer()->getGraphicsItem()) {
                if (graphicsItem) {
                    mpScene->removeItem(graphicsItem);
                }
            }
            pItem->getVectorLayer()->setVisible(false);
        }
        else if (pItem->checkState() == Qt::CheckState::Checked) {
            for (auto graphicsItem : pItem->getVectorLayer()->getGraphicsItem()) {
                if (graphicsItem) {
                    mpScene->addItem(graphicsItem);
                }
            }
            pItem->getVectorLayer()->setVisible(true);
        }
    }
    else if (pItem->getRasterLayer()) {
        if (pItem->checkState() == Qt::CheckState::Unchecked) {
            mpScene->removeItem(pItem->getRasterLayer()->getRasterData());
            pItem->getRasterLayer()->setVisible(false);
        }
        else if (pItem->checkState() == Qt::CheckState::Checked) {
            mpScene->addItem(pItem->getRasterLayer()->getRasterData());
            pItem->getRasterLayer()->setVisible(true);
        }
    }
}

void MyTreeView::performAction1() {
    if (mpChoosedItem) {
        if (mpChoosedItem->getVectorLayer()) {
            mpManager->upVectorLayer(mpChoosedItem->getVectorLayer());
        }
        else if (mpChoosedItem->getRasterLayer()) {
            mpManager->upRasterLayer(mpChoosedItem->getRasterLayer());
        }
    }
}

void MyTreeView::performAction2() {
    if (mpChoosedItem) {
        if (mpChoosedItem->getVectorLayer()) {
            mpManager->downVectorLayer(mpChoosedItem->getVectorLayer());
        }
        else if (mpChoosedItem->getRasterLayer()) {
            mpManager->downRasterLayer(mpChoosedItem->getRasterLayer());
        }
    }
}

void MyTreeView::performAction3() {
    if (mpChoosedItem) {
        // 检查并删除与 mpChoosedItem 关联的图层
        if (mpChoosedItem->getVectorLayer()) {
            mpManager->deleteVectorLayer(mpChoosedItem->getVectorLayer());
            mpChoosedItem->setVectorLayer(nullptr); // 断开关联
        }
        else if (mpChoosedItem->getRasterLayer()) {
            mpManager->deleteRasterLayer(mpChoosedItem->getRasterLayer());
            mpChoosedItem->setRasterLayer(nullptr); // 断开关联
        }

        // 删除列表中的信息
        mpModel->removeRow(mpChoosedItem->row());
        mpChoosedItem = nullptr; // 确保指针不再指向已删除的内存
    }
}


void MyTreeView::performAction4() {
    if (mpChoosedItem) {
        if (mpChoosedItem->getVectorLayer()) {
            mpManager->moveToVectorLayer(mpChoosedItem->getVectorLayer());
        }
        else if (mpChoosedItem->getRasterLayer()) {
            mpManager->moveToRasterLayer(mpChoosedItem->getRasterLayer());
        }
    }
}
void MyTreeView::performAction5() {
    if (mpChoosedItem) {
        if (mpChoosedItem->getVectorLayer()) {
            // 选择颜色
            QColor color = QColorDialog::getColor(Qt::white, nullptr, "选择颜色");

            if (color.isValid()) {
                QBrush brush(color); // 用选择的颜色初始化 QBrush
                QPen pen(color); // 用选择的颜色初始化 QPen
                // 遍历图形项
                for (QGraphicsItem* graphicsItem : mpChoosedItem->getVectorLayer()->getGraphicsItem()) {
                    if (QGraphicsEllipseItem* pItem = dynamic_cast<QGraphicsEllipseItem*>(graphicsItem)) {
                        pItem->setBrush(brush);
                    }
                    else if (QGraphicsPathItem* pItem = dynamic_cast<QGraphicsPathItem*>(graphicsItem)) {
                        pItem->setPen(pen);
                    }
                    else if (QGraphicsPolygonItem* pItem = dynamic_cast<QGraphicsPolygonItem*>(graphicsItem)) {
                        pItem->setBrush(brush);
                    }
                }
            }
        }
    }
}


