#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#include "MyGIS.h"

MyGIS::MyGIS(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    mbIsEditing = false;

    // 创建附加器
    mpFileAppender = new log4cpp::FileAppender("log", "Log.log");
    // 创建布局
    log4cpp::Layout* layout = new log4cpp::BasicLayout();
    mpFileAppender->setLayout(layout);
    // 创建类别
    log4cpp::Category& root = log4cpp::Category::getRoot();
    root.addAppender(mpFileAppender);
    root.setPriority(log4cpp::Priority::INFO);  // 设置日志级别

    // 初始化treeView
    ui.treeView->header()->hide();
    ui.treeView->setIndentation(0);

    // 初始化地图场景
    mpGraphicsScene = new QGraphicsScene(this);
    mpGraphicsScene->setSceneRect(-50000000000, -40000000000, 100000000000, 80000000000);
    ui.graphicsView->setScene(mpGraphicsScene);
    ui.graphicsView->setEditing(false);

    // 初始化图层管理器
    mpLayerManager = new MyLayerManager(ui.treeView, 
        ui.graphicsView, 
        ui.actionStartEditing, 
        ui.actionAttributesView);

    // 初始化工具栏
    ui.actionStartEditing->setEnabled(false);
    ui.actionStopEditing->setEnabled(false);
    ui.actionAttributesView->setEnabled(false);
    ui.actionForward->setEnabled(false);
    ui.actionBackward->setEnabled(false);

    // 隐藏 dock widget
    ui.dockWidget_2->hide();
    ui.dockWidget_3->hide();
    ui.dockWidget_4->hide();

    // 视窗翻转
    mTransform.scale(1, -1);
    ui.graphicsView->setTransform(mTransform);

    // 初始化激活图层指针
    mpOpenedLayer = nullptr;

    // 连接信号和槽
    // 视图窗口相关
    connect(ui.actionLayerManager, &QAction::triggered, this, &MyGIS::openLayerManager);
    connect(ui.actionAttributesView, &QAction::triggered, this, &MyGIS::openAttributesView);
    connect(ui.actionStatisticView, &QAction::triggered, this, &MyGIS::openStatisticView);
    connect(ui.actionHistogramView, &QAction::triggered, this, &MyGIS::openHistogramView);
    // 导入文件相关
    connect(ui.actionVectorLayer, &QAction::triggered, this, &MyGIS::loadVectorLayer);
    connect(ui.actionRasterLayer, &QAction::triggered, this, &MyGIS::loadRasterLayer);
    connect(ui.action_SBRD, &QAction::triggered, this, &MyGIS::loadLargeRaster);
    connect(ui.action_WKT, &QAction::triggered, this, &MyGIS::loadVectorLayer);
    connect(ui.action_Shapefile, &QAction::triggered, this, &MyGIS::loadVectorLayer);
    connect(ui.action_GeoJSON, &QAction::triggered, this, &MyGIS::loadVectorLayer);
    connect(ui.action_Raster, &QAction::triggered, this, &MyGIS::loadRasterLayer);
    // 视窗调整相关
    connect(ui.actionPan, &QAction::triggered, this, &MyGIS::actionPan);
    connect(ui.actionSelect, &QAction::triggered, this, &MyGIS::actionSelect);
    connect(ui.actionZoomIn, &QAction::triggered, this, &MyGIS::actionZoomIn);
    connect(ui.actionZoomOut, &QAction::triggered, this, &MyGIS::actionZoomOut);
    connect(ui.actionLeftRotate, &QAction::triggered, this, &MyGIS::actionLeftRotate);
    connect(ui.actionRightRotate, &QAction::triggered, this, &MyGIS::actionRightRotate);
    //保存相关
    connect(ui.actionScsv, &QAction::triggered, this, &MyGIS::saveAsCsv);
    connect(ui.actionSdb, &QAction::triggered, this, &MyGIS::saveAsDb);
    // 编辑相关
    connect(ui.treeView, &MyTreeView::clicked, this, &MyGIS::onItemClicked);
    connect(ui.actionStartEditing, &QAction::triggered, this, &MyGIS::startEditing);
    connect(ui.actionStopEditing, &QAction::triggered, this, &MyGIS::stopEditing);
    // 打开 Dialog 相关
    connect(ui.actionConvexHull, &QAction::triggered, this, &MyGIS::calConvexHull);
    connect(ui.actionBuffer, &QAction::triggered, this, &MyGIS::calBuffer);
    connect(ui.actionDelaunay, &QAction::triggered, this, &MyGIS::calDelaunay);
    connect(ui.actionColorDisplay, &QAction::triggered, this, &MyGIS::TFColorDisplays);
    connect(ui.actionStatistic, &QAction::triggered, this, &MyGIS::doStatistic);
    connect(ui.actionHistogram, &QAction::triggered, this, &MyGIS::drawHistogram);
    connect(ui.actionMask, &QAction::triggered, this, &MyGIS::rasterMask);
    connect(ui.actionNeighbor, &QAction::triggered, this, &MyGIS::calNeighbor);
    // 保存工程相关
    connect(ui.actionOpenProject, &QAction::triggered, this, &MyGIS::openProject);
    connect(ui.actionSaveProject, &QAction::triggered, this, &MyGIS::saveProject);
}

MyGIS::~MyGIS()
{
    delete mpLayerManager;
    mpLayerManager = nullptr;
}

Ui::MyGISClass MyGIS::getUI() {
    return ui;
}

QGraphicsScene* MyGIS::getScene() {
    return mpGraphicsScene;
}

MyLayerManager* MyGIS::getLayerManager() {
    return mpLayerManager;
}

void MyGIS::openLayerManager() {
    if (ui.dockWidget->isHidden()) {
        ui.dockWidget->show();
    }
}

void MyGIS::openStatisticView() {
    if (ui.dockWidget_3->isHidden()) {
        ui.dockWidget_3->show();
    }
}

void MyGIS::openHistogramView() {
    if (ui.dockWidget_4->isHidden()) {
        ui.dockWidget_4->show();
    }
}

void MyGIS::openAttributesView() {
    if (!mpOpenedLayer) {
        return;
    }

    QTableWidget* pTable = new QTableWidget(ui.dockWidget_2);

    // 设置表头
    QVector<QString> vstrFieldNames = mpOpenedLayer->getFieldNames();
    pTable->setColumnCount(vstrFieldNames.size());
    pTable->setHorizontalHeaderLabels(vstrFieldNames.toList());

    // 填充表格数据
    QVector<MyVectorLayer::AttributeRecord> records = mpOpenedLayer->getRecords();
    for (int row = 0; row < records.size(); row++)
    {
        pTable->insertRow(row);
        for (int col = 0; col < vstrFieldNames.size(); col++)
        {
            QVariant value = records[row].values[col];
            pTable->setItem(row, col, new QTableWidgetItem(value.toString()));
        }
    }

    ui.dockWidget_2->setWidget(pTable);
    if (ui.dockWidget_2->isHidden()) {
        ui.dockWidget_2->show();
    }
    ui.actionAttributesView->setEnabled(false);
}

void MyGIS::actionPan() {
    ui.graphicsView->setEditing(false);
}

void MyGIS::actionSelect() {
    ui.graphicsView->setEditing(true);
}

void MyGIS::actionZoomIn() {
    // 获取当前视图的中心点
    QPointF viewCenter = ui.graphicsView->mapToScene(ui.graphicsView->viewport()->rect().center());
    // 执行缩放
    ui.graphicsView->scale(1.2, 1.2);
    // 缩放后重置视图的中心点
    QPointF newCenter = ui.graphicsView->mapToScene(ui.graphicsView->viewport()->rect().center());
    QPointF deltaCenter = newCenter - viewCenter;
    ui.graphicsView->translate(deltaCenter.x(), deltaCenter.y());
}

void MyGIS::actionZoomOut() {
    // 获取当前视图的中心点
    QPointF viewCenter = ui.graphicsView->mapToScene(ui.graphicsView->viewport()->rect().center());
    // 执行缩放
    ui.graphicsView->scale(1.0 / 1.2, 1.0 / 1.2);
    // 缩放后重置视图的中心点
    QPointF newCenter = ui.graphicsView->mapToScene(ui.graphicsView->viewport()->rect().center());
    QPointF deltaCenter = newCenter - viewCenter;
    ui.graphicsView->translate(deltaCenter.x(), deltaCenter.y());
}

void MyGIS::actionLeftRotate() {
    // 获取当前视图的中心点
    QPointF viewCenter = ui.graphicsView->mapToScene(ui.graphicsView->viewport()->rect().center());
    // 进行旋转，以中心为原点
    qreal angle = -30;  // 旋转角度，顺时针方向为正
    ui.graphicsView->rotate(angle);

    // 旋转后保持视图中心不变
    QPointF newViewCenter = ui.graphicsView->mapToScene(ui.graphicsView->viewport()->rect().center());
    QPointF offset = newViewCenter - viewCenter;
    ui.graphicsView->translate(offset.x(), offset.y());
}

void MyGIS::actionRightRotate() {
    // 获取当前视图的中心点
    QPointF viewCenter = ui.graphicsView->mapToScene(ui.graphicsView->viewport()->rect().center());
    // 进行旋转，以中心为原点
    qreal angle = 30;  // 旋转角度
    ui.graphicsView->rotate(angle);

    // 旋转后保持视图中心不变
    QPointF newViewCenter = ui.graphicsView->mapToScene(ui.graphicsView->viewport()->rect().center());
    QPointF offset = newViewCenter - viewCenter;
    ui.graphicsView->translate(offset.x(), offset.y());
}

void MyGIS::calConvexHull() {
    mpConvexHull = new ConvexHull(this);
    mpConvexHull->addLayer(mpLayerManager->mlVectorLayers);
    mpConvexHull->show();
}

void MyGIS::calBuffer() {
    mpBuffer = new Buffer(this);
    mpBuffer->addLayer(mpLayerManager->mlVectorLayers);
    mpBuffer->show();
}

void MyGIS::calDelaunay() {
    mpDelaunay = new Delaunay(this);
    mpDelaunay->addLayer(mpLayerManager->mlVectorLayers);
    mpDelaunay->show();
}

void MyGIS::doStatistic() {
    mpStatistic = new Statistic(this);
    mpStatistic->addLayer(mpLayerManager->mlVectorLayers);
    mpStatistic->show();
}

void MyGIS::TFColorDisplays() {
    mpTFColorDisplay = new TFColorDisplay(this);
    mpTFColorDisplay->addLayer(mpLayerManager->mlRasterLayers);
    mpTFColorDisplay->show();
}

void MyGIS::drawHistogram() {
    mpHistogram = new Histogram(this);
    mpHistogram->addLayer(mpLayerManager->mlRasterLayers);
    mpHistogram->show();
}

void MyGIS::rasterMask() {
    mpMask = new Mask(this);
    mpMask->addLayer(mpLayerManager->mlVectorLayers, mpLayerManager->mlRasterLayers);
    mpMask->show();
}

void MyGIS::calNeighbor() {
    mpNeighbor = new NeighborhoodStatistics(this);
    mpNeighbor->addLayer(mpLayerManager->mlRasterLayers);
    mpNeighbor->show();
}

void MyGIS::saveAsCsv() {
    mpSaveAsCsv = new SaveAsCsv(this);
    mpSaveAsCsv->addLayer(mpLayerManager->mlVectorLayers);
    mpSaveAsCsv->show();
}

void MyGIS::saveAsDb() {
    mpSaveAsDb = new SaveAsDb(this);
    mpSaveAsDb->addLayer(mpLayerManager->mlVectorLayers);
    mpSaveAsDb->show();
}

void MyGIS::loadVectorLayer() {
    // 打开文件选择对话框，只允许选择特定类型的文件
    QString strFilePath = QFileDialog::getOpenFileName(this, "选择文件", QDir::homePath(), "Vector File (*.shp *.geojson *.csv)");

    if (!strFilePath.isEmpty()) {
        importVector(strFilePath);
    }
}

void MyGIS::loadRasterLayer() {
    QString strFilePath = QFileDialog::getOpenFileName(this, "选择文件", QDir::homePath(), "Raster File (*.tif *.img *.png *.jpg)");

    if (!strFilePath.isEmpty()) {
        importRaster(strFilePath);
    }
}

void MyGIS::loadLargeRaster() {
    QString strFilePath = QFileDialog::getOpenFileName(this, "选择文件", QDir::homePath(), "Raster File (*.tif *.img *.png *.jpg)");

    if (!strFilePath.isEmpty()) {
        importLargeRaster(strFilePath);
    }
}

void MyGIS::onItemClicked(const QModelIndex& index) {
    if (mbIsEditing == false) {
        if (!index.isValid()) {
            return;
        }

        QStandardItemModel* pModel = static_cast<QStandardItemModel*>(ui.treeView->model());
        MyItem* pOpenedItem = static_cast<MyItem*>(pModel->itemFromIndex(index));

        QList<QStandardItem*> lpItems = pModel->findItems(pOpenedItem->text(), Qt::MatchExactly);
        if (lpItems.isEmpty()) {
            mpOpenedLayer = nullptr;
        }
        else {
            ui.actionStartEditing->setEnabled(true);
            ui.actionAttributesView->setEnabled(true);
            mpOpenedLayer = pOpenedItem->getVectorLayer();
        }
    }
    else {
        return;
    }
}

void MyGIS::startEditing() {
    if (!mpOpenedLayer) {
        return;
    }

    log4cpp::Category& log = log4cpp::Category::getRoot();
    log.addAppender(mpFileAppender);
    log.info("开始编辑");

    mpOpenedLayer->setEditing(true);
    ui.actionStartEditing->setEnabled(false);
    ui.actionStopEditing->setEnabled(true);
    ui.graphicsView->setEditing(true);
    mbIsEditing = true;

    for (QGraphicsItem* item : mpOpenedLayer->getGraphicsItem()) {
        if (QGraphicsEllipseItem* ellipse = dynamic_cast<QGraphicsEllipseItem*>(item)) {
            // 将 QGraphicsEllipseItem 替换为 MyPointItem
            QPointF center = ellipse->rect().center();
            MyPointItem* editablePoint = new MyPointItem(center);
            editablePoint->setBrush(ellipse->brush());    // 设置点的填充颜色
            editablePoint->setPen(QPen(Qt::blue, 0.0001)); // 设置点的边框
            editablePoint->setPos(ellipse->pos());        // 保留位置
            mpGraphicsScene->removeItem(ellipse);
            mpGraphicsScene->addItem(editablePoint);
            mlpEditingPoint.append(editablePoint);
        }
        else if (QGraphicsPathItem* line = dynamic_cast<QGraphicsPathItem*>(item)) {
            // 将 QGraphicsLineItem 替换为 MyPathItem
            QPainterPath path = line->path();
            MyPathItem* editablePath = new MyPathItem(path);
            editablePath->setPen(QPen(Qt::blue, 0.00004));  // 设置路径的颜色和宽度
            editablePath->setPos(line->pos());            // 保留位置
            mpGraphicsScene->removeItem(line);
            mpGraphicsScene->addItem(editablePath);
            mlpEditingPath.append(editablePath);
        }
        else if (QGraphicsPolygonItem* polygon = dynamic_cast<QGraphicsPolygonItem*>(item)) {
            // 将 QGraphicsPolygonItem 替换为 MyPolygonItem
            QPolygonF polygonF = polygon->polygon();
            MyPolygonItem* editablePolygon = new MyPolygonItem(polygonF);
            editablePolygon->setPen(QPen(Qt::blue, 0.0001));  // 设置多边形的边框颜色和宽度
            editablePolygon->setBrush(polygon->brush());     // 设置多边形的填充颜色
            editablePolygon->setPos(polygon->pos());         // 保留位置
            mpGraphicsScene->removeItem(polygon);
            mpGraphicsScene->addItem(editablePolygon);
            mlpEditingPolygon.append(editablePolygon);
        }
    }
    mlpEditedItem.clear();
}

void MyGIS::stopEditing() {
	if (!mpOpenedLayer) {
		return;
	}

	mpOpenedLayer->setEditing(false);
	ui.actionStopEditing->setEnabled(false);
	ui.graphicsView->setEditing(false);

	// 清空当前图层的要素
	mpOpenedLayer->clearLayerFeatures();

	// 保存编辑的点
	for (MyPointItem* editingPoint : mlpEditingPoint) {
		QRectF center = editingPoint->rect();
		QPointF pos = editingPoint->pos();

        // 替换为 QGraphicsEllipseItem 并添加到场景
        QGraphicsEllipseItem* editedPoint = new QGraphicsEllipseItem(center);
        editedPoint->setBrush(editingPoint->brush()); // 保留刷
        editedPoint->setPen(QPen(Qt::black, 0.0001));
        editedPoint->setPos(pos); // 保留位置

		// 添加点到当前图层
		mpOpenedLayer->addPointFeature(editedPoint);

		mpGraphicsScene->removeItem(editingPoint);
		mpGraphicsScene->addItem(editedPoint);
		mlpEditedItem.append(editedPoint);
	}

	// 保存编辑的路径
	for (MyPathItem* editingPath : mlpEditingPath) {
        QPainterPath path = editingPath->path();
        // 创建一个QGraphicsPathItem，将构建好的完整路径设置进去
        QGraphicsPathItem* editedPath = new QGraphicsPathItem(path);
        editedPath->setPen(QPen(Qt::black, 0.0001));
        editedPath->setPos(editingPath->pos());     // 保留位置
        // 添加折线到当前图层
        mpOpenedLayer->addLineFeature(editedPath);

        mpGraphicsScene->removeItem(editingPath);
        mpGraphicsScene->addItem(editedPath);
        mlpEditedItem.append(editedPath);
	}

	// 保存编辑的多边形
	for (MyPolygonItem* editingPolygon : mlpEditingPolygon) {
		QPolygonF polygonF = editingPolygon->polygon();
		QVector<QPointF> points = polygonF.toList().toVector();
        // 替换为 QGraphicsPolygonItem 并添加到场景
        QGraphicsPolygonItem* editedPolygon = new QGraphicsPolygonItem(polygonF);
        editedPolygon->setBrush(editingPolygon->brush()); // 保留刷
        editedPolygon->setPen(QPen(Qt::black, 0.0001));
        editedPolygon->setPos(editingPolygon->pos()); // 保留位置
		// 添加多边形到当前图层
		mpOpenedLayer->addPolygonFeature(editedPolygon);

		mpGraphicsScene->removeItem(editingPolygon);
		mpGraphicsScene->addItem(editedPolygon);
		mlpEditedItem.append(editedPolygon);
	}

	// 清空编辑列表
	mlpEditingPath.clear();
	mlpEditingPoint.clear();
	mlpEditingPolygon.clear();

	// 更新图层中的图形项
	mpOpenedLayer->setGraphicsItem(mlpEditedItem);

	// 保存图层数据到 Shapefile
	saveLayerToShapefile(mpOpenedLayer);

	log4cpp::Category& log = log4cpp::Category::getRoot();
	log.addAppender(mpFileAppender);
	log.info("停止编辑");

	mpGraphicsScene->update(); // 更新场景

	ui.actionStartEditing->setEnabled(false); // 禁用编辑按钮
	ui.actionStopEditing->setEnabled(false); // 禁用停止编辑按钮
    mbIsEditing = false;
}

void MyGIS::saveLayerToShapefile(MyVectorLayer* layer) {
    // 打开原始的 Shapefile 文件
    GDALDataset* poDataset = static_cast<GDALDataset*>(GDALOpenEx(layer->getFilePath().toStdString().c_str(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr));
    if (!poDataset) {
        return;
    }

    OGRLayer* poLayer = poDataset->GetLayer(0);
    if (!poLayer) {
        GDALClose(poDataset);
        return;
    }

    // 清除当前 Shapefile 中的所有要素
    OGRFeature* poFeatureToDelete = nullptr;
    poLayer->ResetReading();
    while ((poFeatureToDelete = poLayer->GetNextFeature()) != nullptr) {
        poLayer->DeleteFeature(poFeatureToDelete->GetFID());
        OGRFeature::DestroyFeature(poFeatureToDelete);
    }

    // 逐个添加新要素
    for (QGraphicsItem* item : layer->getGraphicsItem()) {
        OGRFeature* poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());

        // 获取图形的全局位置信息
        QPointF globalPos = item->pos();

        if (QGraphicsEllipseItem* pointItem = dynamic_cast<QGraphicsEllipseItem*>(item)) {
            // 获取局部坐标并加上全局位置信息
            QPointF localPos = pointItem->rect().center();
            QPointF pos = localPos + globalPos;
            OGRPoint point(pos.x(), pos.y());
            poFeature->SetGeometry(&point);
        }
        else if (QGraphicsPathItem* pathItem = dynamic_cast<QGraphicsPathItem*>(item)) {
            QPainterPath path = pathItem->path();
            OGRLineString lineString;

            // 遍历QPainterPath中的每个元素，添加到OGRLineString，并加上全局位置信息
            for (int i = 0; i < path.elementCount(); ++i) {
                QPainterPath::Element element = path.elementAt(i);
                lineString.addPoint(element.x + globalPos.x(), element.y + globalPos.y());
            }

            poFeature->SetGeometry(&lineString);
        }
        else if (QGraphicsPolygonItem* polygonItem = dynamic_cast<QGraphicsPolygonItem*>(item)) {
            QPolygonF polygon = polygonItem->polygon();
            OGRPolygon ogrPolygon;
            OGRLinearRing ring;

            // 遍历多边形的每个顶点，并加上全局位置信息
            for (const QPointF& point : polygon) {
                ring.addPoint(point.x() + globalPos.x(), point.y() + globalPos.y());
            }
            ring.closeRings();
            ogrPolygon.addRing(&ring);
            poFeature->SetGeometry(&ogrPolygon);
        }

        // 将新要素添加到 Shapefile
        if (poLayer->CreateFeature(poFeature) != OGRERR_NONE) {
            // 错误处理
        }

        OGRFeature::DestroyFeature(poFeature);
    }

    GDALClose(poDataset);
}



/*<------------------------------------------------------------------------------------------------------------->*/

void MyGIS::importVector(const QString& strFilePath) {
    // 初始化GDAL
    GDALAllRegister();

    log4cpp::Category& log = log4cpp::Category::getRoot();
    log.addAppender(mpFileAppender);

    // 打开矢量数据集
    GDALDataset* pDataset = static_cast<GDALDataset*>(GDALOpenEx(strFilePath.toStdString().c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
    if (!pDataset) {
        log.warn("无法打开数据集");
        return;
    }

    OGRLayer* poLayer = pDataset->GetLayer(0);
    if (!poLayer) {
        log.warn("无法获取到图层");
        return;
    }

    // 获取字段名称
    QVector<QString> vstrFieldNames;
    int nFieldCount = poLayer->GetLayerDefn()->GetFieldCount();
    for (int i = 0; i < nFieldCount; i++)
    {
        OGRFieldDefn* poFieldDefn = poLayer->GetLayerDefn()->GetFieldDefn(i);
        vstrFieldNames.append(QString(poFieldDefn->GetNameRef()));
    }

    // 获取文件名
    QFileInfo fileInfo(strFilePath);
    QString strFileName = fileInfo.fileName();

    // 创建图形列表
    QList<QGraphicsItem*> vpGraphicsItems;
    
    // 用于获取矢量数据的类型
    GeometryType type = GeometryType::Unknown;
    
    // 创建矢量图层
    MyVectorLayer* pNewLayer = new MyVectorLayer(strFilePath, strFileName, type);
    log.info("创建图层成功");

    // 目标地理坐标系（WGS84）
    OGRSpatialReference oTargetSRS;
    oTargetSRS.SetWellKnownGeogCS("WGS84");

    // 获取原始坐标系
    OGRSpatialReference* poSrcSRS = poLayer->GetSpatialRef();
    OGRSpatialReference ownSrcSRS;
    if (poSrcSRS == nullptr) {
        poSrcSRS = &ownSrcSRS;
        poSrcSRS->SetWellKnownGeogCS("WGS84");
    }

    // 创建坐标转换器
    OGRCoordinateTransformation* poCT = OGRCreateCoordinateTransformation(poSrcSRS, &oTargetSRS);
    if (poCT == nullptr) {
        GDALClose(pDataset);
        return;
    }

    OGRFeature* poFeature = nullptr;
    poLayer->ResetReading();

    QRectF boundingRect;

    while ((poFeature = poLayer->GetNextFeature()) != nullptr) {
        MyVectorLayer::AttributeRecord record;
        // 获取每条记录的属性值
        for (int i = 0; i < nFieldCount; i++)
        {
            QVariant value(QString(poFeature->GetFieldAsString(i)));
            record.values.append(value);
        }

        // 向属性表中添加记录
        pNewLayer->addRecord(record);
        OGRGeometry* geometry = poFeature->GetGeometryRef();
        if (!geometry) {
            log.warn("无法获取到几何");
            OGRFeature::DestroyFeature(poFeature);
            continue;
        }

        // 转换几何图形到目标SRS
        if (poCT != nullptr) {
            geometry->transform(poCT);
        }

        OGRwkbGeometryType geomType = wkbFlatten(geometry->getGeometryType());
        switch (geomType) {
        case wkbPolygon: {
            QList<QGraphicsItem*> vpPartOfItems = processPolygon(static_cast<OGRPolygon*>(geometry), mpGraphicsScene);
            vpGraphicsItems.append(vpPartOfItems);
            type = GeometryType::Polygon;
            break;
        }
        case wkbMultiPolygon: {
            QList<QGraphicsItem*> vpPartOfItems = processMultiPolygon(static_cast<OGRMultiPolygon*>(geometry), mpGraphicsScene);
            vpGraphicsItems.append(vpPartOfItems);
            type = GeometryType::Polygon;
            break;
        }
        case wkbLineString: {
            QList<QGraphicsItem*> vpPartOfItems = processLineString(static_cast<OGRLineString*>(geometry), mpGraphicsScene);
            vpGraphicsItems.append(vpPartOfItems);
            type = GeometryType::Line;
            break;
        }
        case wkbMultiLineString: {
            QList<QGraphicsItem*> vpPartOfItems = processMultiLineString(static_cast<OGRMultiLineString*>(geometry), mpGraphicsScene);
            vpGraphicsItems.append(vpPartOfItems);
            type = GeometryType::Line;
            break;
        }
        case wkbPoint: {
            QGraphicsItem* pPartOfItem = processPoint(static_cast<OGRPoint*>(geometry), mpGraphicsScene);
            vpGraphicsItems.push_back(pPartOfItem);
            type = GeometryType::Point;
            break;
        }
        case wkbMultiPoint: {
            QList<QGraphicsItem*> vpPartOfItems = processMultiPoint(static_cast<OGRMultiPoint*>(geometry), mpGraphicsScene);
            vpGraphicsItems.append(vpPartOfItems);
            type = GeometryType::Point;
            break;
        }
        default:
            break;
        }

        // 更新边界矩形
        for (const auto& item : vpGraphicsItems) {
            boundingRect = boundingRect.isNull() ? item->boundingRect() : boundingRect.united(item->boundingRect());
        }
        OGRFeature::DestroyFeature(poFeature);
    }
    log.info("几何创建成功");

    // 设置图层
    pNewLayer->setGraphicsItem(vpGraphicsItems);
    pNewLayer->setFieldNames(vstrFieldNames);
    pNewLayer->setType(type);
    mpLayerManager->addVectorLayer(pNewLayer);
    mpLayerManager->mlVectorLayers.append(strFilePath);

    // 调整视窗
    if (!boundingRect.isNull()) {
        ui.graphicsView->fitInView(boundingRect, Qt::KeepAspectRatio);
    }

    // 关闭Shapefile数据集
    GDALClose(pDataset);

    // 释放坐标转换器
    if (poCT != nullptr) {
        OCTDestroyCoordinateTransformation(poCT);
    }
    return;
}




/*<------------------------------------------------------------------------------------------------------------->*/

void MyGIS::importRaster(const QString& strFilePath) {
    // 初始化GDAL
    GDALAllRegister();
    
    log4cpp::Category& log = log4cpp::Category::getRoot();
    log.addAppender(mpFileAppender);

    // 打开栅格文件
    GDALDataset* poDataset = static_cast<GDALDataset*>(GDALOpen(strFilePath.toStdString().c_str(), GA_ReadOnly));
    if (!poDataset) {
        log.warn("无法打开栅格数据集");
        return;
    }
    else {
        log.info("打开栅格数据集");
    }

    if (poDataset->GetRasterCount() < 3) {
        log.info("目标栅格的波段数小于3");
    }

    // 使用 QFileInfo 获取文件名
    QFileInfo fileInfo(strFilePath);
    QString strFileName = fileInfo.fileName();

    // 获取栅格大小
    int nXSize = poDataset->GetRasterXSize();
    int nYSize = poDataset->GetRasterYSize();

    GDALRasterBand* poRedBand = poDataset->GetRasterBand(1);
    GDALRasterBand* poGreenBand = poDataset->GetRasterBand(2);
    GDALRasterBand* poBlueBand = poDataset->GetRasterBand(3);

    //检查数据是否有效
    if (!poRedBand || !poGreenBand || !poBlueBand) {
        log.warn("获取某一波段错误");
        log.info("尝试只获取一个波段");

        QVector<uint8_t> vRedData(static_cast<size_t>(nXSize) * static_cast<size_t>(nYSize));

        poRedBand->RasterIO(GF_Read, 0, 0, nXSize, nYSize, vRedData.data(), nXSize, nYSize, GDT_Byte, 0, 0);

        QImage* pImage = new QImage(nXSize, nYSize, QImage::Format_Grayscale8);
        // 使用 OpenMP 并行化像素处理
#pragma omp parallel for
        for (int y = 0; y < nYSize; ++y) {
            for (int x = 0; x < nXSize; ++x) {
                int value = vRedData[static_cast<size_t>(y) * static_cast<size_t>(nXSize) + static_cast<size_t>(x)];
                pImage->setPixel(x, y, qRgb(value, value, value));
            }
        }
        log.info("Image设置成功");

        // 获取仿射变换参数
        double adfGeoTransform[6];
        if (poDataset->GetGeoTransform(adfGeoTransform) == CE_None) {
            double originX = adfGeoTransform[0];  // 左上角X
            double originY = adfGeoTransform[3];  // 左上角Y
            double pixelWidth = adfGeoTransform[1];  // 每个像素的宽度
            double pixelHeight = adfGeoTransform[5];  // 每个像素的高度，通常为负值

            QGraphicsPixmapItem* pPixmapItem = mpGraphicsScene->addPixmap(QPixmap::fromImage(*pImage));

            // 设定图片的位置和缩放比例
            pPixmapItem->setPos(originX, originY);
            pPixmapItem->setScale(pixelWidth);
            pPixmapItem->setTransform(mTransform);

            MyRasterLayer* pNewLayer = new MyRasterLayer(strFilePath, strFileName, nXSize, nYSize);
            log.info("图层创建成功");
            pNewLayer->setRasterData(pPixmapItem);
            pNewLayer->setOriginX(originX);
            pNewLayer->setOriginY(originY);
            pNewLayer->setPixelWidth(pixelWidth);
            pNewLayer->setPixelHeight(pixelHeight);
            mpLayerManager->addRasterLayer(pNewLayer);

            // 获取地理范围
            QRectF boundingRect(QPointF(originX, originY + nYSize * pixelHeight),
                QSizeF(nXSize * pixelWidth, nYSize * std::abs(pixelHeight)));
            // 视窗调整
            ui.graphicsView->fitInView(boundingRect, Qt::KeepAspectRatio);
        }
        else {
            log.warn("获取仿射变换参数失败");
        }

        GDALClose(poDataset);
        return;
    }
    else
    {
        log.info("获取波段");

        QVector<uint8_t> vRedData(static_cast<size_t>(nXSize) * static_cast<size_t>(nYSize));
        QVector<uint8_t> vGreenData(static_cast<size_t>(nXSize) * static_cast<size_t>(nYSize));
        QVector<uint8_t> vBlueData(static_cast<size_t>(nXSize) * static_cast<size_t>(nYSize));

        poRedBand->RasterIO(GF_Read, 0, 0, nXSize, nYSize, vRedData.data(), nXSize, nYSize, GDT_Byte, 0, 0);
        poGreenBand->RasterIO(GF_Read, 0, 0, nXSize, nYSize, vGreenData.data(), nXSize, nYSize, GDT_Byte, 0, 0);
        poBlueBand->RasterIO(GF_Read, 0, 0, nXSize, nYSize, vBlueData.data(), nXSize, nYSize, GDT_Byte, 0, 0);

        QImage* pImage = new QImage(nXSize, nYSize, QImage::Format_RGB32);
        // 使用 OpenMP 并行化像素处理
#pragma omp parallel for
        for (int y = 0; y < nYSize; ++y) {
            for (int x = 0; x < nXSize; ++x) {
                // 在使用索引时进行强制类型转换
                int r = vRedData[static_cast<size_t>(y) * static_cast<size_t>(nXSize) + static_cast<size_t>(x)];
                int g = vGreenData[static_cast<size_t>(y) * static_cast<size_t>(nXSize) + static_cast<size_t>(x)];
                int b = vBlueData[static_cast<size_t>(y) * static_cast<size_t>(nXSize) + static_cast<size_t>(x)];
                pImage->setPixel(x, y, qRgb(r, g, b));
            }
        }
        log.info("Image设置成功");

        // 获取仿射变换参数
        double adfGeoTransform[6];
        if (poDataset->GetGeoTransform(adfGeoTransform) == CE_None) {
            double originX = adfGeoTransform[0];  // 左上角X
            double originY = adfGeoTransform[3];  // 左上角Y
            double pixelWidth = adfGeoTransform[1];  // 每个像素的宽度
            double pixelHeight = adfGeoTransform[5];  // 每个像素的高度，通常为负值

            QGraphicsPixmapItem* pPixmapItem = mpGraphicsScene->addPixmap(QPixmap::fromImage(*pImage));

            // 设定图片的位置和缩放比例
            pPixmapItem->setPos(originX, originY);
            pPixmapItem->setScale(pixelWidth);
            pPixmapItem->setTransform(mTransform);

            MyRasterLayer* pNewLayer = new MyRasterLayer(strFilePath, strFileName, nXSize, nYSize);
            log.info("图层创建成功");
            pNewLayer->setRasterData(pPixmapItem);
            pNewLayer->setOriginX(originX);
            pNewLayer->setOriginY(originY);
            pNewLayer->setPixelWidth(pixelWidth);
            pNewLayer->setPixelHeight(pixelHeight);
            mpLayerManager->addRasterLayer(pNewLayer);
            mpLayerManager->mlRasterLayers.append(strFilePath);

            // 获取地理范围
            QRectF boundingRect(QPointF(originX, originY + nYSize * pixelHeight),
                QSizeF(nXSize * pixelWidth, nYSize * std::abs(pixelHeight)));
            // 视窗调整
            ui.graphicsView->fitInView(boundingRect, Qt::KeepAspectRatio);
        }
        else {
            log.warn("获取仿射变换参数失败");
        }

        GDALClose(poDataset);
        return;
    }
}




/*<------------------------------------------------------------------------------------------------------------->*/

void MyGIS::importLargeRaster(const QString& strFilePath) {
	// 初始化GDAL
	log4cpp::Category& log = log4cpp::Category::getRoot();
	log.addAppender(mpFileAppender);
	GDALAllRegister();

	// 打开栅格文件
	GDALDataset* poDataset = static_cast<GDALDataset*>(GDALOpen(strFilePath.toStdString().c_str(), GA_ReadOnly));
	if (!poDataset) {
		log.warn("Failed to open raster file.");
		return;
	}

	int nBands = poDataset->GetRasterCount();
	if (nBands == 0) {
		GDALClose(poDataset);
		log.warn("No raster bands found.");
		return;
	}
	// 使用 QFileInfo 获取文件名
	QFileInfo fileInfo(strFilePath);
	QString strFileName = fileInfo.fileName();  // 这里定义 strFileName

	// 获取栅格的宽和高
	int nXSize = poDataset->GetRasterXSize();
	int nYSize = poDataset->GetRasterYSize();

	// 设置分块大小
	int blockSizeX = 512;
	int blockSizeY = 512;

	// 为每个波段创建数据存储空间
	std::vector<QVector<uint8_t>> bandData(nBands, QVector<uint8_t>(blockSizeX * blockSizeY));

	// 动态分配 QImage 对象
	QImage* pImage = new QImage(nXSize, nYSize, QImage::Format_RGB32);

	// 使用分块方式读取栅格数据
	for (int y = 0; y < nYSize; y += blockSizeY) {
		int rowsToRead = min(blockSizeY, nYSize - y);
		for (int x = 0; x < nXSize; x += blockSizeX) {
			int colsToRead = min(blockSizeX, nXSize - x);

			// 读取每个波段的分块数据
			for (int b = 0; b < nBands; ++b) {
				GDALRasterBand* poBand = poDataset->GetRasterBand(b + 1);
				poBand->RasterIO(GF_Read, x, y, colsToRead, rowsToRead,
					bandData[b].data(), colsToRead, rowsToRead, GDT_Byte, 0, 0);
			}

			// 将波段数据填充到图像中
			for (int row = 0; row < rowsToRead; ++row) {
				for (int col = 0; col < colsToRead; ++col) {
					int r = bandData[0][row * colsToRead + col];
					int g = (nBands > 1) ? bandData[1][row * colsToRead + col] : r;
					int b = (nBands > 2) ? bandData[2][row * colsToRead + col] : r;
					// 打印一些像素值进行调试
					if (row == 0 && col == 0) {
						log.warn("First pixel values: R=" + std::to_string(r) + ", G=" + std::to_string(g) + ", B=" + std::to_string(b));
					}
					pImage->setPixel(x + col, y + row, qRgb(r, g, b));
				}
			}
		}
	}

	// 获取仿射变换参数
	double adfGeoTransform[6];
	double originX = 0.0;
	double originY = 0.0;
	double pixelWidth = 1.0;
	double pixelHeight = 1.0;

	if (poDataset->GetGeoTransform(adfGeoTransform) == CE_None) {
		// 如果存在地理变换信息
		originX = adfGeoTransform[0];  // 左上角X
		originY = adfGeoTransform[3];  // 左上角Y
		pixelWidth = adfGeoTransform[1];  // 每个像素的宽度
		pixelHeight = adfGeoTransform[5];  // 每个像素的高度，通常为负值

		log.info("GeoTransform found, using geographic coordinates.");
	}
	else {
		// 没有地理变换信息，使用默认的像素坐标
		log.warn("No GeoTransform found, using pixel coordinates.");
	}

	// 创建QGraphicsPixmapItem并添加到场景
	QGraphicsPixmapItem* pPixmapItem = mpGraphicsScene->addPixmap(QPixmap::fromImage(*pImage));

	// 设置图片的位置和缩放比例
	pPixmapItem->setPos(originX, originY);
	pPixmapItem->setScale(pixelWidth);
	pPixmapItem->setTransform(mTransform);

	// 创建MyRasterLayer对象并添加到图层管理器
	MyRasterLayer* pNewLayer = new MyRasterLayer(strFilePath, strFileName, nXSize, nYSize);
	pNewLayer->setRasterData(pPixmapItem);
	pNewLayer->setOriginX(originX);
	pNewLayer->setOriginY(originY);
	pNewLayer->setPixelWidth(pixelWidth);
	pNewLayer->setPixelHeight(pixelHeight);
	mpLayerManager->addRasterLayer(pNewLayer);
	mpLayerManager->mlRasterLayers.append(strFilePath);

	// 获取地理范围或使用默认范围
	QRectF boundingRect(QPointF(originX, originY + nYSize * pixelHeight),
		QSizeF(nXSize * pixelWidth, nYSize * std::abs(pixelHeight)));

	// 调整视图窗口
	ui.graphicsView->fitInView(boundingRect, Qt::KeepAspectRatio);

	GDALClose(poDataset);
}





/*<--------------------------------------------------------------------------------------------------------------->*/

QList<QGraphicsItem*> MyGIS::processPolygon(OGRPolygon* polygon, QGraphicsScene* scene) {
    QList<QGraphicsItem*> polygonItems;
    QPolygonF qPolygon;

    OGRLinearRing* exteriorRing = polygon->getExteriorRing();
    if (exteriorRing) {
        int numPoints = exteriorRing->getNumPoints();
        for (int i = 0; i < numPoints; i++) {
            double x = exteriorRing->getX(i);
            double y = exteriorRing->getY(i);
            qPolygon << QPointF(x, y);
        }
    }

    QPen pen(Qt::black);
    pen.setWidth(0.2);
    QBrush brush(Qt::lightGray);
    QGraphicsItem* polygonItem = scene->addPolygon(qPolygon, pen, brush);
    polygonItems.push_back(polygonItem);

    for (int i = 0; i < polygon->getNumInteriorRings(); i++) {
        OGRLinearRing* interiorRing = polygon->getInteriorRing(i);
        QPolygonF qInteriorPolygon;
        int numPoints = interiorRing->getNumPoints();
        for (int j = 0; j < numPoints; j++) {
            double x = interiorRing->getX(j);
            double y = interiorRing->getY(j);
            qInteriorPolygon << QPointF(x, y);
        }
        QGraphicsItem* interiorPolygonItem = scene->addPolygon(qInteriorPolygon, pen, brush);

        polygonItems.append(interiorPolygonItem);
    }

    return polygonItems;
}

QList<QGraphicsItem*> MyGIS::processMultiPolygon(OGRMultiPolygon* multiPolygon, QGraphicsScene* scene) {
    QList<QGraphicsItem*> multiPolygonItems;
    for (int i = 0; i < multiPolygon->getNumGeometries(); i++) {
        OGRPolygon* polygon = static_cast<OGRPolygon*>(multiPolygon->getGeometryRef(i));
        QList<QGraphicsItem*> polygonItems = processPolygon(polygon, scene);
        multiPolygonItems.append(polygonItems);
    }
    return multiPolygonItems;
}

QList<QGraphicsItem*> MyGIS::processLineString(OGRLineString* lineString, QGraphicsScene* scene) {
    QList<QGraphicsItem*> pathItems;
    QPainterPath path;
    int numPoints = lineString->getNumPoints();
    if (numPoints > 0) {
        path.moveTo(lineString->getX(0), lineString->getY(0));
        for (int i = 1; i < numPoints; i++) {
            path.lineTo(lineString->getX(i), lineString->getY(i));
        }
    }

    QPen pen(Qt::black);
    pen.setWidth(0.2);
    QGraphicsPathItem* pathItem = scene->addPath(path, pen);
    pathItems.push_back(pathItem);

    return pathItems;
}

QList<QGraphicsItem*> MyGIS::processMultiLineString(OGRMultiLineString* multiLineString, QGraphicsScene* scene) {
    QList<QGraphicsItem*> multiPathItems;
    for (int i = 0; i < multiLineString->getNumGeometries(); i++) {
        OGRLineString* lineString = static_cast<OGRLineString*>(multiLineString->getGeometryRef(i));
        QList<QGraphicsItem*> pathItems = processLineString(lineString, scene);
        multiPathItems.append(pathItems);
    }
    return multiPathItems;
}

QGraphicsItem* MyGIS::processPoint(OGRPoint* point, QGraphicsScene* scene) {
    QPen pen(Qt::black);
    pen.setWidth(0.2);
    QBrush brush(Qt::lightGray);
    double x = point->getX();
    double y = point->getY();
    return scene->addEllipse(x - 0.0001, y - 0.0001, 0.0002, 0.0002, pen, brush);
}

QList<QGraphicsItem*> MyGIS::processMultiPoint(OGRMultiPoint* multiPoint, QGraphicsScene* scene) {
    QList<QGraphicsItem*> multiPointItems;
    for (int i = 0; i < multiPoint->getNumGeometries(); i++) {
        OGRPoint* point = static_cast<OGRPoint*>(multiPoint->getGeometryRef(i));
        QGraphicsItem* multiPointItem = processPoint(point, scene);
        multiPointItems.push_back(multiPointItem);
    }
    return multiPointItems;
}




/*<--------------------------------------------------------------------------------------------------------------->*/

void MyGIS::openProject() {
    log4cpp::Category& log = log4cpp::Category::getRoot();
    log.addAppender(mpFileAppender);
    QString strFilePath = QFileDialog::getOpenFileName(this, "Open Project", "", "Project Files (*.gproj)");
    if (strFilePath.isEmpty()) {
        return;
    }

    QFile file(strFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Failed to open project file.");
        log.warn("无法打开工程文件");
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(data));
    QJsonObject project = doc.object();

    // 加载矢量图层
    QJsonArray vecLayersArray = project["vecLayers"].toArray();
    for (int i = 0; i < vecLayersArray.size(); ++i) {
        QJsonObject layerObj = vecLayersArray[i].toObject();
        QString fileName = layerObj["vecFileName"].toString();
        importVector(fileName);
    }
    log.info("矢量图层打开成功");
    // 加载栅格图层
    QJsonArray rasLayersArray = project["rasLayers"].toArray();
    for (int i = 0; i < rasLayersArray.size(); ++i) {
        QJsonObject layerObj = rasLayersArray[i].toObject();
        QString fileName = layerObj["rasFileName"].toString();
        importRaster(fileName);
    }
    log.info("栅格图层打开成功");
    // 恢复视图设置
    QJsonObject viewSettings = project["viewSettings"].toObject();
    double scale = viewSettings["scale"].toDouble(1.0);
    ui.graphicsView->resetTransform(); // 先重置变换矩阵
    ui.graphicsView->setTransform(mTransform);
    ui.graphicsView->scale(scale, scale);
    log.info("视图恢复成功");
    file.close();
}

void MyGIS::saveProject() {
    log4cpp::Category& log = log4cpp::Category::getRoot();
    log.addAppender(mpFileAppender);
    QString filePath = QFileDialog::getSaveFileName(this, "Save Project", "", "Project Files (*.gproj)");
    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Error", "Failed to save project file.");
        log.warn("无法打开工程文件");
        return;
    }

    QJsonObject project;
    // 保存矢量图层信息
    QJsonArray vecLayersArray;
    for (int i = 0; i < mpLayerManager->mlVectorLayers.size(); ++i) {
        QJsonObject vecLayerObj;
        vecLayerObj["vecFileName"] = mpLayerManager->mlVectorLayers[i]; // 保存图层文件路径
        //vecLayerObj["vecZValue"] = mpLayerManager->mlpVectorLayer[i]->getZValue();
        vecLayersArray.append(vecLayerObj);
    }
    project["vecLayers"] = vecLayersArray;
    log.info("矢量图层保存成功");
    // 保存栅格图层信息
    QJsonArray rasLayersArray;
    for (int i = 0; i < mpLayerManager->mlRasterLayers.size(); ++i) {
        QJsonObject rasLayerObj;
        rasLayerObj["rasFileName"] = mpLayerManager->mlRasterLayers[i]; // 保存图层文件路径
        //rasLayerObj["rasZValue"] = mpLayerManager->mlpRasterLayer[i]->getZValue();
        rasLayersArray.append(rasLayerObj);
    }
    project["rasLayers"] = rasLayersArray;
    log.info("栅格图层保存成功");
    // 保存视图设置
    QJsonObject viewSettings;
    viewSettings["scale"] = ui.graphicsView->transform().m11(); // 获取当前缩放级别
    project["viewSettings"] = viewSettings;
    log.info("视图设置保存成功");
    QJsonDocument doc(project);
    file.write(doc.toJson());
    file.close();
}
