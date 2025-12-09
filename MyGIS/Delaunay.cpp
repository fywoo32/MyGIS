#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#include "Delaunay.h"
#include "MyGIS.h"

Delaunay::Delaunay(MyGIS* mainWindow, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	mpMainWindow = mainWindow;

	connect(ui.toolButton, &QToolButton::clicked, this, &Delaunay::actionOpenFile);
	connect(ui.pushButton, &QPushButton::clicked, this, &Delaunay::actionYes);
	connect(ui.pushButton_2, &QPushButton::clicked, this, &Delaunay::actionNo);
}

Delaunay::~Delaunay()
{}

void Delaunay::actionOpenFile() {
    // 打开文件保存对话框
    QString strFilePath = QFileDialog::getSaveFileName(
        nullptr,
        "Save File",
        "",
        "Shapefile (*.shp)"
    );
    ui.lineEdit->setText(strFilePath);
}

void Delaunay::actionYes() {
    generateDelaunayTriangulation(ui.comboBox->currentText(), ui.lineEdit->text(), ui.doubleSpinBox->value(), ui.checkBox->checkState());
    this->close();
}

void Delaunay::actionNo() {
    this->close();
}

void Delaunay::addLayer(const QStringList& strlLayers) {
    ui.comboBox->addItems(strlLayers);
}

void Delaunay::generateDelaunayTriangulation(const QString& strInputPath, const QString& strOutputPath, double dfTolerance, bool bOnlyEdges) {
    // 初始化GDAL库
    GDALAllRegister();
    log4cpp::Category& log = log4cpp::Category::getRoot();
    log.addAppender(mpMainWindow->mpFileAppender);
    log.info("开始计算Delaunay三角网");

    // 打开输入矢量数据文件
    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(strInputPath.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
    if (poDS == nullptr) {
        log.warn("无法打开数据集");
        return;
    }

    // 获取输入图层
    OGRLayer* poLayer = poDS->GetLayer(0);
    if (poLayer == nullptr) {
        GDALClose(poDS);
        log.warn("无法获取到图层");
        return;
    }

    // 创建输出SHP文件
    const char* pszDriverName = "ESRI Shapefile";
    GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
    if (poDriver == nullptr) {
        log.warn("无法获取SHP驱动");
        GDALClose(poDS);
        return;
    }

    GDALDataset* poOutDS = poDriver->Create(strOutputPath.toStdString().c_str(), 0, 0, 0, GDT_Unknown, NULL);
    if (poOutDS == nullptr) {
        log.warn("无法创建输出SHP文件");
        GDALClose(poDS);
        return;
    }

    // 创建图层，使用与输入图层相同的空间参考
    OGRSpatialReference* poSRS = poLayer->GetSpatialRef();
    OGRLayer* poOutLayer = poOutDS->CreateLayer("delaunay", poSRS, bOnlyEdges ? wkbLineString : wkbPolygon, NULL);
    if (poOutLayer == nullptr) {
        log.warn("无法创建输出图层");
        GDALClose(poOutDS);
        GDALClose(poDS);
        return;
    }

    // 初始化一个空的几何体指针
    OGRGeometry* poMergedGeometry = nullptr;

    // 遍历输入图层中的每个要素并合并几何体
    OGRFeature* poFeature;
    while ((poFeature = poLayer->GetNextFeature()) != nullptr) {
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        if (poGeometry != nullptr) {
            if (poMergedGeometry == nullptr) {
                // 如果 poMergedGeometry 为空，则将第一个几何体赋值给它
                poMergedGeometry = poGeometry->clone();
            }
            else {
                // 否则将当前几何体与现有合并几何体进行联合
                OGRGeometry* poNewMergedGeometry = poMergedGeometry->Union(poGeometry);
                OGRGeometryFactory::destroyGeometry(poMergedGeometry);
                poMergedGeometry = poNewMergedGeometry;
            }
        }
        OGRFeature::DestroyFeature(poFeature);
    }

    // 对合并后的几何体计算Delaunay三角网
    if (poMergedGeometry != nullptr) {
        OGRGeometry* poDelaunay = poMergedGeometry->DelaunayTriangulation(dfTolerance, bOnlyEdges);
        if (poDelaunay != nullptr) {
            // 创建输出要素并设置几何体
            OGRFeature* poOutFeature = OGRFeature::CreateFeature(poOutLayer->GetLayerDefn());
            poOutFeature->SetGeometry(poDelaunay);

            // 添加要素到输出图层
            if (poOutLayer->CreateFeature(poOutFeature) != OGRERR_NONE) {
                log.warn("无法创建要素到输出图层");
            }

            // 清理
            OGRFeature::DestroyFeature(poOutFeature);
            OGRGeometryFactory::destroyGeometry(poDelaunay);
        }
        else {
            log.warn("Delaunay三角网生成失败");
        }

        // 清理合并后的几何体
        OGRGeometryFactory::destroyGeometry(poMergedGeometry);
    }
    else {
        log.warn("没有可用的几何体进行Delaunay三角网计算");
    }

    // 关闭数据集
    GDALClose(poOutDS);
    GDALClose(poDS);
    mpMainWindow->importVector(strOutputPath);
}

