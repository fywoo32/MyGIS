#pragma once

#include <QDialog>

#include "gdal_priv.h"
#include "gdal_alg.h"
#include "gdal_alg_priv.h"
#include "gdal_utils.h"
#include "ogr_spatialref.h"
#include "ogr_geometry.h"
#include "ogr_api.h"
#include "cpl_conv.h"
#include "cpl_error.h"
#include "omp.h"
#include "ui_Mask.h"

class MyGIS;
class Mask : public QDialog
{
	Q_OBJECT

public:
	Mask(MyGIS* mainWindow, QWidget *parent = nullptr);
	~Mask();
	void addLayer(const QStringList& strlLayers1, const QStringList& strlLayers2);
	void maskRaster(const QString& strInputMask, const QString& strInputRas, const QString& strOutput);
	void maskVector(const QString& strInputMask, const QString& strInputRas, const QString& strOutput);
	bool isVectorGDAL(const QString& strFileName);
	bool isRasterGDAL(const QString& strFileName);

public slots:
	void actionOpenFile();
	void actionYes();
	void actionNo();

private:
	Ui::MaskClass ui;
	MyGIS* mpMainWindow;
};
