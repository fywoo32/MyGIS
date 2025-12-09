#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#include "Statistic.h"
#include "MyGIS.h"

Statistic::Statistic(MyGIS* mainWindow, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
    mpMainWindow = mainWindow;

    connect(ui.pushButton, &QPushButton::clicked, this, &Statistic::actionYes);
    connect(ui.pushButton_2, &QPushButton::clicked, this, &Statistic::actionNo);
}

Statistic::~Statistic()
{}

void Statistic::actionYes() {
    QTableWidget* pTable = new QTableWidget(mpMainWindow->getUI().dockWidget_3);
    pTable->setColumnCount(4);
    pTable->setHorizontalHeaderLabels(QStringList() << "要素类型" << "数量" << "面积" << "周长/长度");

    analyzeVectorData(ui.comboBox->currentText(), pTable);

    mpMainWindow->getUI().dockWidget_3->setWidget(pTable);
    mpMainWindow->getUI().dockWidget_3->show();
    this->close();
}

void Statistic::actionNo() {
    this->close();
}

void Statistic::addLayer(const QStringList& strlLayers) {
    ui.comboBox->addItems(strlLayers);
}

void Statistic::analyzeVectorData(const QString& strFilePath, QTableWidget* pTable) {
    GDALAllRegister();

    log4cpp::Category& log = log4cpp::Category::getRoot();
    log.addAppender(mpMainWindow->mpFileAppender);
    log.info("开始统计矢量要素");

    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(strFilePath.toStdString().c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
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


    OGRFeature* poFeature;
    QMap<QString, int> featureCounts;
    QMap<QString, double> featureAreas;
    QMap<QString, double> featurePerimeters;

    poLayer->ResetReading();
    while ((poFeature = poLayer->GetNextFeature()) != nullptr) {
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        if (poGeometry != nullptr) {
            QString geometryType = QString::fromStdString(poGeometry->getGeometryName());

            double area = 0.0;
            double perimeter = 0.0;

            if (wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon) {
                OGRPolygon* poPolygon = (OGRPolygon*)poGeometry;
                area = poPolygon->get_Area();
                perimeter = poPolygon->getExteriorRing()->get_Length();
            }
            else if (wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPolygon) {
                OGRMultiPolygon* poMultiPolygon = (OGRMultiPolygon*)poGeometry;
                for (int i = 0; i < poMultiPolygon->getNumGeometries(); i++) {
                    OGRPolygon* poPolygon = (OGRPolygon*)poMultiPolygon->getGeometryRef(i);
                    area += poPolygon->get_Area();
                    perimeter += poPolygon->getExteriorRing()->get_Length();
                }
            }
            else if (wkbFlatten(poGeometry->getGeometryType()) == wkbLineString) {
                OGRLineString* poLineString = (OGRLineString*)poGeometry;
                perimeter = poLineString->get_Length();
            }
            else if (wkbFlatten(poGeometry->getGeometryType()) == wkbMultiLineString) {
                OGRMultiLineString* poMultiLineString = (OGRMultiLineString*)poGeometry;
                for (int i = 0; i < poMultiLineString->getNumGeometries(); i++) {
                    OGRLineString* poLineString = (OGRLineString*)poMultiLineString->getGeometryRef(i);
                    perimeter += poLineString->get_Length();
                }
            }

            featureCounts[geometryType] += 1;
            featureAreas[geometryType] += area;
            featurePerimeters[geometryType] += perimeter;
        }
        OGRFeature::DestroyFeature(poFeature);
    }
    log.info("要素统计完毕");
    GDALClose(poDS);

    int row = 0;
    log.info("统计结果输出到表格");
    for (auto it = featureCounts.begin(); it != featureCounts.end(); ++it) {
        pTable->insertRow(row);
        pTable->setItem(row, 0, new QTableWidgetItem(it.key()));
        pTable->setItem(row, 1, new QTableWidgetItem(QString::number(it.value())));
        pTable->setItem(row, 2, new QTableWidgetItem(QString::number(featureAreas[it.key()], 'f', 6)));
        pTable->setItem(row, 3, new QTableWidgetItem(QString::number(featurePerimeters[it.key()], 'f', 6)));
        row++;
    }
}
