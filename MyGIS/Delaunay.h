#pragma once

#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include <ogr_geometry.h>
#include <ogrsf_frmts.h>
#include <QDialog>
#include "ui_Delaunay.h"

class MyGIS;
class Delaunay : public QDialog
{
	Q_OBJECT

public:
	Delaunay(MyGIS* mainWindow, QWidget *parent = nullptr);
	~Delaunay();
	void addLayer(const QStringList& strlLayers);
	void generateDelaunayTriangulation(const QString& strInputPath, const QString& strOutputPath, double dfTolerance, bool bOnlyEdges);

public slots:
	void actionOpenFile();
	void actionYes();
	void actionNo();

private:
	Ui::DelaunayClass ui;
	MyGIS* mpMainWindow;
};
