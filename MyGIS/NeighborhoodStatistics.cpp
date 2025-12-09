#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#include "NeighborhoodStatistics.h"
#include "MyGIS.h"

NeighborhoodStatistics::NeighborhoodStatistics(MyGIS* mainWindow, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	mpMainWindow = mainWindow;

    ui.spinBox->setMinimum(1);
    ui.comboBox_2->addItem("Mean");
    ui.comboBox_2->addItem("Max");
    ui.comboBox_2->addItem("Min");

    connect(ui.toolButton, &QToolButton::clicked, this, &NeighborhoodStatistics::actionOpenFile);
    connect(ui.pushButton, &QPushButton::clicked, this, &NeighborhoodStatistics::actionYes);
    connect(ui.pushButton_2, &QPushButton::clicked, this, &NeighborhoodStatistics::actionNo);
}

NeighborhoodStatistics::~NeighborhoodStatistics()
{}

void NeighborhoodStatistics::actionOpenFile() {
    // 打开文件保存对话框
    QString strFilePath = QFileDialog::getSaveFileName(
        nullptr,
        "Save File",
        "",
        "GeoTiff (*.tif);;All Files (*)"
    );
    ui.lineEdit->setText(strFilePath);
}

void NeighborhoodStatistics::actionYes() {
    HowToStatistic how = HowToStatistic::UNKNOW;
    if (ui.comboBox_2->currentText() == "Mean") {
        how = HowToStatistic::MEAN;
    }
    else if (ui.comboBox_2->currentText() == "Max") {
        how = HowToStatistic::MAX;
    }
    else if (ui.comboBox_2->currentText() == "Min") {
        how = HowToStatistic::MIN;
    }
    neighborhoodStatistics(
        ui.comboBox->currentText(),
        ui.lineEdit->text(),
        ui.spinBox->value(),
        how
    );
    this->close();
}

void NeighborhoodStatistics::actionNo() {
    this->close();
}

void NeighborhoodStatistics::addLayer(const QStringList& strlLayers) {
	ui.comboBox->addItems(strlLayers);
}

float NeighborhoodStatistics::calculateMean(const std::vector<float>& vfValues) {
    if (vfValues.empty()) return 0.0f;
    float fSum = std::accumulate(vfValues.begin(), vfValues.end(), 0.0f);
    return fSum / vfValues.size();
}

float NeighborhoodStatistics::calculateMax(const std::vector<float>& vfValues) {
    return *std::max_element(vfValues.begin(), vfValues.end());
}

float NeighborhoodStatistics::calculateMin(const std::vector<float>& vfValues) {
    return *std::min_element(vfValues.begin(), vfValues.end());
}

void NeighborhoodStatistics::neighborhoodStatistics(const QString& strInputRas, const QString& strOutputRas, int nSize, HowToStatistic how) {
    GDALAllRegister();

    log4cpp::Category& log = log4cpp::Category::getRoot();
    log.addAppender(mpMainWindow->mpFileAppender);
    log.info("邻域统计");

    GDALDataset* poDataset = static_cast<GDALDataset*>(GDALOpen(strInputRas.toStdString().c_str(), GA_ReadOnly));
    if (poDataset == nullptr) {
        log.warn("无法打开栅格数据集");
        return;
    }
    log.info("打开栅格数据集");

    GDALRasterBand* poBand = poDataset->GetRasterBand(1);
    int nXSize = poBand->GetXSize();
    int nYSize = poBand->GetYSize();
    int halfSize = nSize / 2;

    // 创建输出文件
    log.info("创建驱动");
    GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    log.info("创建输出数据集");
    GDALDataset* poOutput = poDriver->Create(strOutputRas.toStdString().c_str(), nXSize, nYSize, 1, GDT_Float32, nullptr);

    // 获取仿射变换参数
    log.info("获取仿射变换参数");
    double adfGeoTransform[6];
    poDataset->GetGeoTransform(adfGeoTransform);
    poOutput->SetGeoTransform(adfGeoTransform);

    // 复制投影信息
    const char* pszProjection = poDataset->GetProjectionRef();
    if (pszProjection) {
        poOutput->SetProjection(pszProjection);
    }

    // 为每个像素分配缓冲区
    float* pfBuffer = new float[nXSize];

    log.info("开始进行邻域统计");

    // 遍历每一行
    for (int y = 0; y < nYSize; ++y) {
        int startY = (std::max)(0, y - halfSize);
        int endY = (std::min)(nYSize - 1, y + halfSize);

        // 读取窗口数据
        std::vector<float> windowData((endY - startY + 1) * nXSize);
        poBand->RasterIO(GF_Read, 0, startY, nXSize, endY - startY + 1,
            windowData.data(), nXSize, endY - startY + 1, GDT_Float32, 0, 0);

        for (int x = 0; x < nXSize; ++x) {
            int startX = (std::max)(0, x - halfSize);
            int endX = (std::min)(nXSize - 1, x + halfSize);

            std::vector<float> values;

            for (int iy = 0; iy <= (endY - startY); ++iy) {
                for (int ix = startX; ix <= endX; ++ix) {
                    values.push_back(windowData[iy * nXSize + ix]);
                }
            }

            if (how == HowToStatistic::MEAN) {
                pfBuffer[x] = calculateMean(values);
            }
            else if (how == HowToStatistic::MAX) {
                pfBuffer[x] = calculateMax(values);
            }
            else if (how == HowToStatistic::MIN) {
                pfBuffer[x] = calculateMin(values);
            }
        }

        // 写入数据
        poOutput->GetRasterBand(1)->RasterIO(GF_Write, 0, y, nXSize, 1, pfBuffer, nXSize, 1, GDT_Float32, 0, 0);
    }

    delete[] pfBuffer;
    GDALClose(poOutput);
    GDALClose(poDataset);
    mpMainWindow->importRaster(strOutputRas);
}

