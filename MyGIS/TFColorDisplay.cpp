#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#include "TFColorDisplay.h"
#include "MyGIS.h"

TFColorDisplay::TFColorDisplay(MyGIS* mainWindow, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	mpMainWindow = mainWindow;

    ui.spinBox->setMinimum(1);
    ui.spinBox_2->setMinimum(1);
    ui.spinBox_3->setMinimum(1);
    initialiseSpinBox(ui.comboBox->currentText());

    connect(ui.pushButton_2, &QPushButton::clicked, this, &TFColorDisplay::actionYes);
    connect(ui.pushButton, &QPushButton::clicked, this, &TFColorDisplay::actionNo);
    connect(ui.comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TFColorDisplay::actionReread);
}

TFColorDisplay::~TFColorDisplay()
{}

void TFColorDisplay::actionYes() {
    displayCompositeImage(ui.comboBox->currentText(), 
        ui.spinBox->value(), 
        ui.spinBox_2->value(), 
        ui.spinBox_3->value());
    this->close();
}

void TFColorDisplay::actionNo() {
    this->close();
}

void TFColorDisplay::actionReread() {
    initialiseSpinBox(ui.comboBox->currentText());
}

void TFColorDisplay::addLayer(const QStringList& strlLayers) {
	ui.comboBox->addItems(strlLayers);
}

void TFColorDisplay::initialiseSpinBox(const QString& strFilePath) {
    GDALAllRegister();  // 注册所有GDAL驱动

    GDALDataset* poDataset = static_cast<GDALDataset*>(GDALOpen(strFilePath.toStdString().c_str(), GA_ReadOnly));
    if (!poDataset) {
        return;
    }
    int nCount = poDataset->GetRasterCount();
    ui.spinBox->setMaximum(nCount);
    ui.spinBox_2->setMaximum(nCount);
    ui.spinBox_3->setMaximum(nCount);

    GDALClose(poDataset);
}

void TFColorDisplay::displayCompositeImage(const QString& strFilePath, int nRedBand, int nGreenBand, int nBlueBand) {
    GDALAllRegister();  // 注册所有GDAL驱动

    log4cpp::Category& log = log4cpp::Category::getRoot();
    log.addAppender(mpMainWindow->mpFileAppender);
    log.info("进行波段组合");

    GDALDataset* poDataset = static_cast<GDALDataset*>(GDALOpen(strFilePath.toStdString().c_str(), GA_ReadOnly));
    if (!poDataset) {
        log.warn("无法打开栅格数据集");
        return;
    }
    log.info("打开栅格数据集");

    // 使用 QFileInfo 获取文件名
    QFileInfo fileInfo(strFilePath);
    QString strFileName = fileInfo.fileName();

    // 获取栅格大小
    int nXSize = poDataset->GetRasterXSize();
    int nYSize = poDataset->GetRasterYSize();

    // 创建用于存储RGB波段数据的数组
    QVector<uchar> vRedBandData(nXSize * nYSize);
    QVector<uchar> vGreenBandData(nXSize * nYSize);
    QVector<uchar> vBlueBandData(nXSize * nYSize);

    int nCount = poDataset->GetRasterCount();
    ui.spinBox->setMaximum(nCount);
    ui.spinBox_2->setMaximum(nCount);
    ui.spinBox_3->setMaximum(nCount);

    // 读取指定波段的数据
    GDALRasterBand* poRedBand = poDataset->GetRasterBand(nRedBand);
    poRedBand->RasterIO(GF_Read, 0, 0, nXSize, nYSize, vRedBandData.data(), nXSize, nYSize, GDT_Byte, 0, 0);

    GDALRasterBand* poGreenBand = poDataset->GetRasterBand(nGreenBand);
    poGreenBand->RasterIO(GF_Read, 0, 0, nXSize, nYSize, vGreenBandData.data(), nXSize, nYSize, GDT_Byte, 0, 0);

    GDALRasterBand* poBlueBand = poDataset->GetRasterBand(nBlueBand);
    poBlueBand->RasterIO(GF_Read, 0, 0, nXSize, nYSize, vBlueBandData.data(), nXSize, nYSize, GDT_Byte, 0, 0);

    //检查数据是否有效
    if (!poRedBand || !poGreenBand || !poBlueBand) {
        log.warn("获取某一波段错误");
        QMessageBox::critical(this, "错误", "波段不存在");
        GDALClose(poDataset);
        return;
    }
    log.info("获取波段");

    // 组合波段生成RGB图像
    QImage* pImage = new QImage(nXSize, nYSize, QImage::Format_RGB32);
#pragma omp parallel for
    for (int y = 0; y < nYSize; ++y) {
        for (int x = 0; x < nXSize; ++x) {
            int idx = y * nXSize + x;
            pImage->setPixel(x, y, qRgb(vRedBandData[idx], vGreenBandData[idx], vBlueBandData[idx]));
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

        QGraphicsPixmapItem* pPixmapItem = mpMainWindow->getScene()->addPixmap(QPixmap::fromImage(*pImage));
        QTransform transform;
        transform.scale(1, -1);

        // 设定图片的位置和缩放比例
        pPixmapItem->setPos(originX, originY);
        pPixmapItem->setScale(pixelWidth);
        pPixmapItem->setTransform(transform);

        MyRasterLayer* pNewLayer = new MyRasterLayer(strFilePath, strFileName, nXSize, nYSize);
        pNewLayer->setRasterData(pPixmapItem);
        pNewLayer->setOriginX(originX);
        pNewLayer->setOriginY(originY);
        pNewLayer->setPixelWidth(pixelWidth);
        pNewLayer->setPixelHeight(pixelHeight);
        mpMainWindow->getLayerManager()->addRasterLayer(pNewLayer);
        mpMainWindow->getLayerManager()->mlRasterLayers.append(strFilePath);

        // 获取地理范围
        QRectF boundingRect(QPointF(originX, originY + nYSize * pixelHeight),
            QSizeF(nXSize * pixelWidth, nYSize * std::abs(pixelHeight)));
        // 视窗调整
        mpMainWindow->getUI().graphicsView->fitInView(boundingRect, Qt::KeepAspectRatio);
    }
    else {
        log.warn("获取仿射变换参数失败");
    }

    GDALClose(poDataset);
    return;
}
