#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#include "Buffer.h"
#include "MyGIS.h"

Buffer::Buffer(MyGIS* mainWindow, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	mpMainWindow = mainWindow;

    connect(ui.toolButton, &QToolButton::clicked, this, &Buffer::actionOpenFile);
    connect(ui.pushButton, &QPushButton::clicked, this, &Buffer::actionYes);
    connect(ui.pushButton_2, &QPushButton::clicked, this, &Buffer::actionNo);
}

Buffer::~Buffer()
{}

void Buffer::actionOpenFile() {
    // 打开文件保存对话框
    QString strFilePath = QFileDialog::getSaveFileName(
        nullptr,
        "Save File",
        "",
        "Shapefile (*.shp);;All Files (*)"
    );
    ui.lineEdit->setText(strFilePath);
}

void Buffer::actionYes() {
    createBuffer(ui.comboBox->currentText(), ui.lineEdit->text(), ui.doubleSpinBox->value()/100);
    this->close();
}

void Buffer::actionNo() {
    this->close();
}

void Buffer::addLayer(const QStringList& strlLayers) {
	ui.comboBox->addItems(strlLayers);
}

void Buffer::createBuffer(const QString& strInput, const QString& strOutput, double dDistance) {
    // 注册所有驱动
    GDALAllRegister();

    log4cpp::Category& log = log4cpp::Category::getRoot();
    log.addAppender(mpMainWindow->mpFileAppender);
    log.info("开始计算缓冲区");

    // 打开输入矢量数据集
    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(strInput.toStdString().c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
    if (poDS == nullptr) {
        log.warn("无法打开数据集");
        return;
    }
    log.info("打开数据集");

    // 获取图层
    OGRLayer* poLayer = poDS->GetLayer(0);
    if (poLayer == nullptr) {
        GDALClose(poDS);
        log.warn("无法获取到图层");
        return;
    }
    log.info("获取到图层");

    // 创建输出矢量数据集
    GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
    if (poDriver == nullptr) {
        GDALClose(poDS);
        log.warn("输出驱动创建失败");
        return;
    }
    log.info("创建驱动");

    GDALDataset* poODS = poDriver->Create(strOutput.toStdString().c_str(), 0, 0, 0, GDT_Unknown, nullptr);
    if (poODS == nullptr) {
        GDALClose(poDS);
        log.warn("输出数据集创建失败");
        return;
    }
    log.info("创建输出数据集");

    // 创建图层
    OGRLayer* poOutLayer = poODS->CreateLayer(poLayer->GetName(), nullptr, wkbPolygon, nullptr);
    if (poOutLayer == nullptr) {
        GDALClose(poDS);
        GDALClose(poODS);
        log.warn("图层创建失败");
        return;
    }
    log.info("创建图层");

    // 初始化一个空的几何对象，用于存储合并后的缓冲区
    OGRGeometry* poMergedBuffer = nullptr;

    // 读取输入图层的每个要素并计算缓冲区
    OGRFeature* poFeature = nullptr;
    poLayer->ResetReading();

    while ((poFeature = poLayer->GetNextFeature()) != nullptr) {
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        if (poGeometry != nullptr) {
            OGRGeometry* poBuffer = poGeometry->Buffer(dDistance);
            if (poBuffer != nullptr && wkbFlatten(poBuffer->getGeometryType()) == wkbPolygon) {
                if (poMergedBuffer == nullptr) {
                    // 初始时，直接赋值
                    poMergedBuffer = poBuffer;
                }
                else {
                    // 合并缓冲区
                    OGRGeometry* poTemp = poMergedBuffer->Union(poBuffer);
                    delete poBuffer;  // 删除临时缓冲区几何对象
                    delete poMergedBuffer;
                    poMergedBuffer = poTemp;
                }
            }
        }
        OGRFeature::DestroyFeature(poFeature);
    }
    log.info("创建缓冲区图形");

    if (poMergedBuffer != nullptr && wkbFlatten(poMergedBuffer->getGeometryType()) == wkbPolygon) {
        // 将合并后的缓冲区保存到输出 Shapefile
        OGRFeature* poOutFeature = OGRFeature::CreateFeature(poOutLayer->GetLayerDefn());
        poOutFeature->SetGeometry(poMergedBuffer);
        if (poOutLayer->CreateFeature(poOutFeature) != OGRERR_NONE) {
            log.warn("要素创建错误");
        }
        OGRFeature::DestroyFeature(poOutFeature);
    }
    log.info("Shapefile输出成功");

    // 清理资源
    if (poMergedBuffer != nullptr) {
        delete poMergedBuffer;
    }
    // 关闭数据集
    GDALClose(poDS);
    GDALClose(poODS);
    mpMainWindow->importVector(strOutput);
    return;
}
