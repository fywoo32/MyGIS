#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#include "ConvexHull.h"
#include "MyGIS.h"

ConvexHull::ConvexHull(MyGIS* mainWindow, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
    mpMainWindow = mainWindow;

    connect(ui.toolButton, &QToolButton::clicked, this, &ConvexHull::actionOpenFile);
    connect(ui.pushButton, &QPushButton::clicked, this, &ConvexHull::actionYes);
    connect(ui.pushButton_2, &QPushButton::clicked, this, &ConvexHull::actionNo);
}

ConvexHull::~ConvexHull()
{}

void ConvexHull::actionOpenFile() {
    // 打开文件保存对话框
    QString strFilePath = QFileDialog::getSaveFileName(
        nullptr,
        "Save File",
        "",
        "Shapefile (*.shp)"
    );
    ui.lineEdit->setText(strFilePath);
}

void ConvexHull::actionYes() {
    calculateConvexHull(ui.comboBox->currentText(), ui.lineEdit->text());
    this->close();
}

void ConvexHull::actionNo() {
    this->close();
}

void ConvexHull::addLayer(const QStringList& strlLayers) {
    ui.comboBox->addItems(strlLayers);
}

void ConvexHull::calculateConvexHull(const QString& strLayer, const QString& strFilePath) {
    GDALAllRegister();
    log4cpp::Category& log = log4cpp::Category::getRoot();
    log.addAppender(mpMainWindow->mpFileAppender);
    log.info("开始计算凸包");

    GDALDataset* poDS = static_cast<GDALDataset*>(GDALOpenEx(strLayer.toStdString().c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
    if (poDS == nullptr) {
        log.warn("无法打开数据集");
        return;
    }
    log.info("打开数据集");

    OGRLayer* poLayer = poDS->GetLayer(0);
    if (poLayer == nullptr) {
        GDALClose(poDS);
        log.warn("无法获取到图层");
        return;
    }
    log.info("获取到图层");

    std::vector<OGRGeometry*> geometries;
    OGRFeature* poFeature;
    poLayer->ResetReading();

    // Collect all geometries first
    while ((poFeature = poLayer->GetNextFeature()) != nullptr) {
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        if (poGeometry != nullptr) {
            geometries.push_back(poGeometry->clone());
        }
        OGRFeature::DestroyFeature(poFeature);
    }

    log.info("几何体收集完成");

    // Parallelize geometry merging in batches using OpenMP
    OGRGeometry* poMergedGeometry = nullptr;
    int batchSize = 100;  // Adjust batch size based on dataset size
#pragma omp parallel for
    for (int i = 0; i < geometries.size(); i += batchSize) {
        OGRGeometry* poBatchGeometry = nullptr;
        for (int j = i; j < (std::min)(i + batchSize, (int)geometries.size()); ++j) {
            if (poBatchGeometry == nullptr) {
                poBatchGeometry = geometries[j]->clone();
            }
            else {
                OGRGeometry* poTemp = poBatchGeometry->Union(geometries[j]);
                delete poBatchGeometry;
                poBatchGeometry = poTemp;
            }
        }

        // Combine batch results (thread-safe merge)
#pragma omp critical
        {
            if (poMergedGeometry == nullptr) {
                poMergedGeometry = poBatchGeometry->clone();
            }
            else {
                OGRGeometry* poTemp = poMergedGeometry->Union(poBatchGeometry);
                delete poMergedGeometry;
                poMergedGeometry = poTemp;
            }
        }
        delete poBatchGeometry;
    }

    log.info("几何体合并完成");

    // Calculate the convex hull of the merged geometries
    OGRGeometry* poConvexHull = poMergedGeometry ? poMergedGeometry->ConvexHull() : nullptr;
    delete poMergedGeometry;
    log.info("凸包计算成功");

    if (poConvexHull != nullptr) {
        const char* pszDriverName = "ESRI Shapefile";
        GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
        if (poDriver == nullptr) {
            delete poConvexHull;
            GDALClose(poDS);
            log.warn("输出驱动创建失败");
            return;
        }

        GDALDataset* poOutDS = poDriver->Create(strFilePath.toStdString().c_str(), 0, 0, 0, GDT_Unknown, nullptr);
        if (poOutDS == nullptr) {
            delete poConvexHull;
            GDALClose(poDS);
            log.warn("输出数据集创建失败");
            return;
        }

        OGRLayer* poOutLayer = poOutDS->CreateLayer(poLayer->GetName(), nullptr, wkbPolygon, nullptr);
        if (poOutLayer == nullptr) {
            delete poConvexHull;
            GDALClose(poOutDS);
            GDALClose(poDS);
            log.warn("图层创建失败");
            return;
        }

        OGRFeature* poOutFeature = OGRFeature::CreateFeature(poOutLayer->GetLayerDefn());
        poOutFeature->SetGeometry(poConvexHull);
        poOutLayer->CreateFeature(poOutFeature);

        OGRFeature::DestroyFeature(poOutFeature);
        GDALClose(poOutDS);
        log.info("Shapefile输出成功");
    }

    delete poConvexHull;
    GDALClose(poDS);
    mpMainWindow->importVector(strFilePath);
}
