#include "MyGIS.h"
#include "SaveAsCsv.h"

SaveAsCsv::SaveAsCsv(MyGIS* mainWindow, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	mpMainWindow = mainWindow;

	connect(ui.toolButton, &QToolButton::clicked, this, &SaveAsCsv::actionOpenFile);
	connect(ui.pushButton, &QPushButton::clicked, this, &SaveAsCsv::actionYes);
	connect(ui.pushButton_2, &QPushButton::clicked, this, &SaveAsCsv::actionNo);
}

SaveAsCsv::~SaveAsCsv()
{}

void SaveAsCsv::actionOpenFile() {
    // 打开文件保存对话框
    QString strFilePath = QFileDialog::getSaveFileName(
        nullptr,
        "Save File",
        "",
        "WKT (*.csv)"
    );
    ui.lineEdit->setText(strFilePath);
}

void SaveAsCsv::actionYes() {
    saveCSV(ui.comboBox->currentText(), ui.lineEdit->text());
    this->close();
}

void SaveAsCsv::actionNo() {
    this->close();
}

void SaveAsCsv::addLayer(const QStringList& strlLayers) {
    ui.comboBox->addItems(strlLayers);
}

void SaveAsCsv::saveCSV(const QString& strInputPath, const QString& strOutputPath) {
    // 注册所有的驱动
    GDALAllRegister();

    // 打开矢量数据集
    GDALDataset* poDataset = (GDALDataset*)GDALOpenEx(strInputPath.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
    if (poDataset == nullptr) {
        std::cerr << "Failed to open vector file." << std::endl;
        return;
    }

    // 获取图层
    OGRLayer* poLayer = poDataset->GetLayer(0);
    if (poLayer == nullptr) {
        std::cerr << "Failed to get layer from dataset." << std::endl;
        GDALClose(poDataset);
        return;
    }

    // 创建CSV文件
    std::ofstream csvFile(strOutputPath.toStdString());
    if (!csvFile.is_open()) {
        std::cerr << "Failed to create CSV file." << std::endl;
        GDALClose(poDataset);
        return;
    }

    // 写入CSV头部
    csvFile << "WKT,Id,OBJECTID,Shape_Leng,Shape_Area" << std::endl;

    // 遍历所有要素
    OGRFeature* poFeature;
    poLayer->ResetReading();
    while ((poFeature = poLayer->GetNextFeature()) != nullptr) {
        // 获取WKT表示的几何图形
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        char* pszWKT = nullptr;
        if (poGeometry != nullptr) {
            poGeometry->exportToWkt(&pszWKT);
        }

        // 获取属性信息
        int id = poFeature->GetFID();
        int objectId = poFeature->GetFieldAsInteger("OBJECTID");
        double shapeLeng = poFeature->GetFieldAsDouble("Shape_Leng");
        double shapeArea = poFeature->GetFieldAsDouble("Shape_Area");

        // 处理 WKT 字符串
        std::string wktString = pszWKT ? pszWKT : "";
        // 替换引号为两个引号，以符合CSV格式
        std::replace(wktString.begin(), wktString.end(), '"', '\"');

        // 使用引号包裹WKT值
        std::string quotedWktString = "\"" + wktString + "\"";

        // 将数据写入CSV文件
        csvFile << quotedWktString << ",";
        csvFile << id << ",";
        csvFile << objectId << ",";
        csvFile << shapeLeng << ",";
        csvFile << shapeArea << std::endl;

        // 释放WKT字符串
        if (pszWKT) {
            CPLFree(pszWKT);
        }

        // 释放要素
        OGRFeature::DestroyFeature(poFeature);
    }

    // 关闭CSV文件
    csvFile.close();

    // 关闭数据集
    GDALClose(poDataset);
    mpMainWindow->importVector(strOutputPath);
}
