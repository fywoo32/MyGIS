#pragma once

#include <QDialog>
#include <QGraphicsItem>
#include <QList>
#include <QPointF>
#include <QPolygonF>
#include <iostream>
#include <fstream>

#include "ui_ConvexHull.h"
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "ogr_geometry.h"
#include "omp.h"

class MyGIS;
class ConvexHull : public QDialog
{
	Q_OBJECT

public:
	ConvexHull(MyGIS* mainWindow, QWidget *parent = nullptr);
	~ConvexHull();
	void addLayer(const QStringList& strlLayers);
	void calculateConvexHull(const QString& strLayer, const QString& strFilePath);

public slots:
	void actionOpenFile();
	void actionYes();
	void actionNo();

private:
	Ui::ConvexHullClass ui;
	MyGIS* mpMainWindow;
};
