#pragma once

#include <QtWidgets/QMainWindow>
#include <QVector>
#include <QGraphicsScene>
#include <QFileDialog>
#include <QTableWidget>
#include <QTextStream>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <algorithm>

#include "ui_MyGIS.h"
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "ogr_geometry.h"
#include "ogr_spatialref.h"
#include "omp.h"
#include "proj.h"

#include <log4cpp/Category.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>

#include "MyGraphicsItem.h"
#include "MyLayerManager.h"
#include "Storage.h"
#include "ConvexHull.h"
#include "Buffer.h"
#include "Delaunay.h"
#include "Statistic.h"
#include "TFColorDisplay.h"
#include "Histogram.h"
#include "Mask.h"
#include "NeighborhoodStatistics.h"
#include "SaveAsCsv.h"
#include "SaveAsDb.h"

#include <QSqlDatabase>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <libpq-fe.h>

class MyGIS : public QMainWindow
{
    Q_OBJECT

public:
    MyGIS(QWidget *parent = nullptr);
    ~MyGIS();

    Ui::MyGISClass getUI();
    QGraphicsScene* getScene();
    MyLayerManager* getLayerManager();
    MyVectorLayer* mpOpenedLayer;            // 激活图层
    QList<MyPolygonItem*> mlpEditingPolygon; // 编辑中的面要素
    QList<MyPathItem*> mlpEditingPath;       // 编辑中的线要素
    QList<MyPointItem*> mlpEditingPoint;     // 编辑中的点要素
    QList<QGraphicsItem*> mlpEditedItem;     // 编辑后的矢量图形
    log4cpp::Appender* mpFileAppender;
    bool mbIsEditing;

    void importVector(const QString& strFilePath);       // 导入矢量图层
    void importRaster(const QString& strFilePath);       // 导入栅格图层
    void importLargeRaster(const QString& strFilePath);  // 导入超大栅格
    void saveLayerToShapefile(MyVectorLayer* layer);

protected slots:
    void openLayerManager();               // 打开图层管理器
    void openStatisticView();              // 打开属性表
    void openHistogramView();
    void openAttributesView();
    void actionPan();
    void actionSelect();
    void actionZoomIn();
    void actionZoomOut();
    void actionLeftRotate();
    void actionRightRotate();
    void loadVectorLayer();                // 导入矢量图层
    void loadRasterLayer();                // 导入栅格图层
    void loadLargeRaster();
    void onItemClicked(const QModelIndex& index);
    void startEditing();               // 开始编辑
    void stopEditing();                // 停止编辑
    void calConvexHull();                 // 凸包计算
    void calBuffer();                     // 缓冲区计算
    void calDelaunay();                   
    void doStatistic();                  // 统计分析
    void TFColorDisplays();             // 真假彩色显示
    void drawHistogram();                  // 绘制直方图
    void rasterMask();                       // 按掩膜提取
    void calNeighbor();                   // 邻域统计
    void openProject();
    void saveProject();
    void saveAsCsv();
    void saveAsDb();

private:
    Ui::MyGISClass ui;
    QTransform mTransform;                   // 变换矩阵（调整坐标系统）
    QGraphicsScene* mpGraphicsScene;         // 地图画布
    MyLayerManager* mpLayerManager;          // 图层管理器

    QList<QGraphicsItem*> processPolygon(OGRPolygon* polygon, QGraphicsScene* scene);
    QList<QGraphicsItem*> processMultiPolygon(OGRMultiPolygon* multiPolygon, QGraphicsScene* scene);
    QList<QGraphicsItem*> processLineString(OGRLineString* lineString, QGraphicsScene* scene);
    QList<QGraphicsItem*> processMultiLineString(OGRMultiLineString* multiLineString, QGraphicsScene* scene);
    QGraphicsItem* processPoint(OGRPoint* point, QGraphicsScene* scene);
    QList<QGraphicsItem*> processMultiPoint(OGRMultiPoint* multiPoint, QGraphicsScene* scene);

    // 子窗口指针
    ConvexHull* mpConvexHull;
    Buffer* mpBuffer;
    Delaunay* mpDelaunay;
    Statistic* mpStatistic;
    TFColorDisplay* mpTFColorDisplay;
    Histogram* mpHistogram;
    Mask* mpMask;
    NeighborhoodStatistics* mpNeighbor;
    SaveAsCsv* mpSaveAsCsv;
    SaveAsDb* mpSaveAsDb;
};
