#pragma once

#include <QString>
#include <QGraphicsItem>
#include <QPen>
#include <QBrush>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>

#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "ogr_geometry.h"

enum class GeometryType {
    Point,
    Line,
    Polygon,
    Unknown
};

class MyLayer {
public:
    MyLayer(const QString& strFilePath, const QString& strName, bool bVisible = true)
        : mstrFilePath(strFilePath), mstrName(strName), mbVisible(bVisible) {}

    ~MyLayer() {}

    QString getFilePath() const { return mstrFilePath; }

    QString getName() const { return mstrName; }
    void setName(const QString& strName) { mstrName = strName; }

    bool isVisible() const { return mbVisible; }
    void setVisible(bool bVisibility) { mbVisible = bVisibility; }

    qreal getZValue() const { return mzValue; }
    void setZValue(qreal zValue) { mzValue = zValue; }

private:
    QString mstrFilePath; // 文件路径
    QString mstrName;     // 图层名称
    bool mbVisible;       // 是否可见
    qreal mzValue;        // Z 轴
};

class MyVectorLayer : public MyLayer {
public:
    struct AttributeRecord
    {
        QVector<QVariant> values;  // 每条记录的属性值
    };

	// 默认构造函数
	MyVectorLayer()
		: MyLayer("", "", true), mType(GeometryType::Unknown) {
		mbEditing = false;
	}

    MyVectorLayer(const QString& strFilePath, const QString& strName, GeometryType type, bool bVisible = true)
        : MyLayer(strFilePath, strName, bVisible), mType(type) {
        mbEditing = false;
    }

    ~MyVectorLayer() {
        // 销毁图形项
        for (QGraphicsItem* pItem : mvpGraphicsItems) {
            if (pItem) {
                delete pItem;  // 释放 QGraphicsItem 指针占用的内存
            }
        }
        mvpGraphicsItems.clear(); // 清空列表
    }

    void setGraphicsItem(QList<QGraphicsItem*> vpGraphicsItems) {
        mvpGraphicsItems = vpGraphicsItems;
        //setZValue(vpGraphicsItems[0]->zValue());
    }
    
    QList<QGraphicsItem*> getGraphicsItem() const{
        return mvpGraphicsItems;
    }

    bool isEditing() const { return mbEditing; }
    void setEditing(bool bEdit) { mbEditing = bEdit; }
    void setType(const GeometryType& type) { mType = type; }
    void setFieldNames(const QVector<QString>& fieldNames) { mvstrFieldNames = fieldNames; }
    void addRecord(const AttributeRecord& record) { mvRecords.append(record); }

    GeometryType getType() const { return mType; }
    QVector<QString> getFieldNames() const { return mvstrFieldNames; }
    QVector<AttributeRecord> getRecords() const { return mvRecords; }
	
    // 清除图层中的所有要素
	void clearLayerFeatures() {
		for (QGraphicsItem* pItem : mvpGraphicsItems) {
			if (pItem) {
				delete pItem;  // 释放 QGraphicsItem 指针占用的内存
			}
		}
		mvpGraphicsItems.clear();  // 清空列表
	}

	void addPointFeature(QGraphicsEllipseItem* pPointItem) {
		mvpGraphicsItems.append(pPointItem);

		// 添加要素到属性记录或其他数据结构（根据需要）
		// 例如：mRecords.append(...);
	}

	void addLineFeature(QGraphicsPathItem* pPathItem) {
        // 将QGraphicsPathItem添加到mvpGraphicsItems
        mvpGraphicsItems.append(pPathItem);

		// 添加要素到属性记录或其他数据结构（根据需要）
		// 例如：mRecords.append(...);
	}

	void addPolygonFeature(QGraphicsPolygonItem* pPolygonItem) {
		mvpGraphicsItems.append(pPolygonItem);

		// 添加要素到属性记录或其他数据结构（根据需要）
		// 例如：mRecords.append(...);
	}

	//void updateGeometryData() {
	//	// 根据 mvpGraphicsItems 更新几何数据
	//	for (QGraphicsItem* item : mvpGraphicsItems) {
	//		if (QGraphicsEllipseItem* ellipse = dynamic_cast<QGraphicsEllipseItem*>(item)) {
	//			QPointF pos = ellipse->pos();
	//			addPointFeature(pos.x(), pos.y());
	//		}
	//		else if (QGraphicsPathItem* line = dynamic_cast<QGraphicsPathItem*>(item)) {
	//			addLineFeature(line);
	//		}
	//		else if (QGraphicsPolygonItem* polygon = dynamic_cast<QGraphicsPolygonItem*>(item)) {
	//			QPolygonF polygonF = polygon->polygon();
	//			QVector<QPointF> points = polygonF.toList().toVector();
	//			addPolygonFeature(points);
	//		}
	//	}
	//}

	void updateLayerData(const QList<QGraphicsItem*>& editedItems) {
		mvpGraphicsItems = editedItems;
		// 可能还需要更新与属性数据相关的信息，例如字段和值
		// 根据你的实际需求来更新数据
	}

private:
    bool mbEditing;       // 是否正在编辑
    GeometryType mType;   // 类型
    QList<QGraphicsItem*> mvpGraphicsItems;
    QVector<QString> mvstrFieldNames;    // 字段名称
    QVector<AttributeRecord> mvRecords;  // 属性记录
};

class MyRasterLayer : public MyLayer {
public:
    MyRasterLayer(const QString& strFilePath, const QString& strName, int nRows, int nCols, bool bVisible = true)
        : MyLayer(strFilePath, strName, bVisible), mnRows(nRows), mnCols(nCols) {
        // 在这里可以添加更多初始化代码
    }

    ~MyRasterLayer() {
        // 销毁栅格图像
        if (mpPixmapItem) {
            delete mpPixmapItem;  // 释放 QGraphicsPixmapItem 指针占用的内存
        }
        mpPixmapItem = nullptr;
    }

    int getRowCount() const { return mnRows; }
    int getColCount() const { return mnCols; }

    QGraphicsPixmapItem* getRasterData() const { return mpPixmapItem; }
    void setRasterData(QGraphicsPixmapItem* pPixmapItem) { 
        mpPixmapItem = pPixmapItem; 
        setZValue(pPixmapItem->zValue());
    }

    void setOriginX(double dOriginX) { mdOriginX = dOriginX; }
    void setOriginY(double dOriginY) { mdOriginY = dOriginY; }
    void setPixelWidth(double dPixelWidth) { mdPixelWidth = dPixelWidth; }
    void setPixelHeight(double dPixelHeight) { mdPixelHeight = dPixelHeight; }

    double getOriginX() { return mdOriginX; }
    double getOriginY() { return mdOriginY; }
    double getPixelWidth() { return mdPixelWidth; }
    double getPixelHeight() { return mdPixelHeight; }

private:
    int mnRows;          // 栅格的行数
    int mnCols;          // 栅格的列数
    QGraphicsPixmapItem* mpPixmapItem;
    double mdOriginX;     // 左上角X
    double mdOriginY;     // 左上角Y
    double mdPixelWidth;  // 每个像素的宽度
    double mdPixelHeight; // 每个像素的高度，通常为负值
};

