#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#include "Histogram.h"
#include "MyGIS.h"

Histogram::Histogram(MyGIS* mainWindow, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	mpMainWindow = mainWindow;

	connect(ui.pushButton, &QPushButton::clicked, this, &Histogram::actionYes);
	connect(ui.pushButton_2, &QPushButton::clicked, this, &Histogram::actionNo);
    connect(ui.toolButton, &QToolButton::clicked, this, &Histogram::actionOpenFile);
}

Histogram::~Histogram()
{}

void Histogram::addLayer(const QStringList& strlLayers) {
	ui.comboBox->addItems(strlLayers);
}

void Histogram::actionOpenFile() {
    // 打开文件保存对话框
    QString strFilePath = QFileDialog::getSaveFileName(
        nullptr,
        "Save File",
        "",
        "GeoTiff (*.tif);;All Files (*)"
    );
    ui.lineEdit->setText(strFilePath);
}

void Histogram::actionYes() {
    QGraphicsScene* pNewScene = new QGraphicsScene(mpMainWindow->getUI().graphicsView_2);
    mpMainWindow->getUI().graphicsView_2->setScene(pNewScene);
    processAndDrawHistogram(ui.comboBox->currentText(), pNewScene, 256);
    HistogramEqualization(ui.comboBox->currentText(), ui.lineEdit->text());
    mpMainWindow->getUI().dockWidget_4->show();
    this->close();
}

void Histogram::actionNo() {
    this->close();
}

void Histogram::processAndDrawHistogram(const QString& strFilename, QGraphicsScene* pScene, int nBins) {
    GDALAllRegister();  // 注册所有GDAL驱动

    log4cpp::Category& log = log4cpp::Category::getRoot();
    log.addAppender(mpMainWindow->mpFileAppender);
    log.info("准备绘制灰度直方图");

    GDALDataset* poDataset = static_cast<GDALDataset*>(GDALOpen(strFilename.toStdString().c_str(), GA_ReadOnly));
    if (!poDataset) {
        log.warn("无法打开栅格数据集");
        return;
    }
    log.info("打开栅格数据集");

    int nWidth = poDataset->GetRasterXSize();
    int nHeight = poDataset->GetRasterYSize();
    GDALRasterBand* poBand = poDataset->GetRasterBand(1);
    log.info("获取波段");

    QVector<uchar> vData(nWidth * nHeight);
    poBand->RasterIO(GF_Read, 0, 0, nWidth, nHeight, vData.data(), nWidth, nHeight, GDT_Byte, 0, 0);

    GDALClose(poDataset);

    // 计算直方图
    QVector<int> vnHistogram(nBins, 0);
    log.info("计算直方图");
#pragma omp parallel
    {
        QVector<int> localHistogram(nBins, 0);

#pragma omp for
        for (int i = 0; i < vData.size(); ++i) {
            localHistogram[vData[i]]++;
        }

#pragma omp critical
        {
            for (int i = 0; i < nBins; ++i) {
                vnHistogram[i] += localHistogram[i];
            }
        }
    }

    log.info("开始绘制灰度直方图");
    // 绘制直方图
    int nBinWidth = 1;  // 每个条形的宽度
    int nMaxCount = *std::max_element(vnHistogram.begin(), vnHistogram.end());

    for (int i = 0; i < vnHistogram.size(); ++i) {
        int nHeight = static_cast<int>(static_cast<double>(vnHistogram[i]) / nMaxCount * 100);
        QGraphicsRectItem* pRect = new QGraphicsRectItem(i * nBinWidth, 100 - nHeight, nBinWidth, nHeight);
        pRect->setBrush(QBrush(Qt::blue));
        pScene->addItem(pRect);
    }
    log.info("绘制结束");
}

void Histogram::HistogramEqualization(const QString& strInputPath, const QString& strOutputPath) {
    GDALAllRegister();

    log4cpp::Category& log = log4cpp::Category::getRoot();
    log.addAppender(mpMainWindow->mpFileAppender);
    log.info("直方图均衡化增强显示");

    // 打开输入栅格
    GDALDataset* poDataset = (GDALDataset*)GDALOpen(strInputPath.toStdString().c_str(), GA_ReadOnly);
    if (poDataset == nullptr) {
        log.warn("无法打开栅格数据集");
        return;
    }
    log.info("打开栅格数据集");

    int nBands = poDataset->GetRasterCount();
    int nXSize = poDataset->GetRasterXSize();
    int nYSize = poDataset->GetRasterYSize();

    log.info("创建驱动");
    GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    log.info("创建输出数据集");
    GDALDataset* poOutputDataset = poDriver->Create(strOutputPath.toStdString().c_str(), nXSize, nYSize, nBands, GDT_Byte, NULL);

    // 获取仿射变换参数
    double adfGeoTransform[6];
    poDataset->GetGeoTransform(adfGeoTransform);
    poOutputDataset->SetGeoTransform(adfGeoTransform);
    if (poDataset->GetProjectionRef()) {
        poOutputDataset->SetProjection(poDataset->GetProjectionRef());
    }

    std::vector<int> histogram(256, 0);  // 假设像素值为0到255的8位灰度图

    log.info("开始计算");
    for (int iBand = 1; iBand <= nBands; iBand++) {
        GDALRasterBand* poBand = poDataset->GetRasterBand(iBand);
        std::vector<unsigned char> imageData(nXSize * nYSize);
        poBand->RasterIO(GF_Read, 0, 0, nXSize, nYSize, imageData.data(), nXSize, nYSize, GDT_Byte, 0, 0);

        // 打印输入数据的最小值和最大值，方便调试
        double dfMin, dfMax;
        poBand->ComputeStatistics(false, &dfMin, &dfMax, nullptr, nullptr, nullptr, nullptr);

        // 计算直方图
        std::fill(histogram.begin(), histogram.end(), 0);
        for (size_t i = 0; i < imageData.size(); i++) {
            histogram[imageData[i]]++;
        }

        // 计算累积分布函数（CDF）
        std::vector<int> cdf(256, 0);
        std::partial_sum(histogram.begin(), histogram.end(), cdf.begin());

        // CDF最小值
        int cdfMin = *std::find_if(cdf.begin(), cdf.end(), [](int value) { return value > 0; });

        // 应用直方图均衡化
        for (size_t i = 0; i < imageData.size(); i++) {
            int pixelValue = imageData[i];
            imageData[i] = static_cast<unsigned char>(255.0 * (cdf[pixelValue] - cdfMin) / (nXSize * nYSize - cdfMin));
        }

        // 将均衡化后的图像数据写入输出栅格
        GDALRasterBand* poOutputBand = poOutputDataset->GetRasterBand(iBand);
        poOutputBand->RasterIO(GF_Write, 0, 0, nXSize, nYSize, imageData.data(), nXSize, nYSize, GDT_Byte, 0, 0);
    }
    log.info("均衡化显示设置完毕");

    GDALClose(poDataset);
    GDALClose(poOutputDataset);
    mpMainWindow->importRaster(strOutputPath);
}


