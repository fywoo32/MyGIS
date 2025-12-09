#pragma once

#include <QDialog>
#include <vector>
#include <algorithm>
#include <numeric>
#include <omp.h>

#include "ui_NeighborhoodStatistics.h"
#include "gdal_priv.h"
#include "cpl_conv.h"

enum class HowToStatistic {
	MEAN,
	MAX,
	MIN,
	UNKNOW
};

class MyGIS;
class NeighborhoodStatistics : public QDialog
{
	Q_OBJECT

public:
	NeighborhoodStatistics(MyGIS* mainWindow, QWidget *parent = nullptr);
	~NeighborhoodStatistics();
	void addLayer(const QStringList& strlLayers);
	float calculateMean(const std::vector<float>& vfValues);
	float calculateMax(const std::vector<float>& vfValues);
	float calculateMin(const std::vector<float>& vfValues);
	void neighborhoodStatistics(const QString& strInputRas, const QString& strOutputRas, int nSize, HowToStatistic how);

public slots:
	void actionOpenFile();
	void actionYes();
	void actionNo();

private:
	Ui::NeighborhoodStatisticsClass ui;
	MyGIS* mpMainWindow;
};
