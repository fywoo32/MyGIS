#pragma once

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <vector>
#include <algorithm>
#include <numeric>

#include "ui_Histogram.h"
#include "omp.h"
#include "gdal_priv.h"

class MyGIS;
class Histogram : public QDialog
{
	Q_OBJECT

public:
	Histogram(MyGIS* mainWindow, QWidget *parent = nullptr);
	~Histogram();
	void addLayer(const QStringList& strlLayers);
	void HistogramEqualization(const QString& strInputPath, const QString& strOutputPath);
	void processAndDrawHistogram(const QString& strFilename, QGraphicsScene* pScene, int nBins);

public slots:
	void actionOpenFile();
	void actionYes();
	void actionNo();

private:
	Ui::HistogramClass ui;
	MyGIS* mpMainWindow;
	QVector<uchar> mvBuffer;
};
