#pragma once

#include <QDialog>
#include <iostream>

#include "ui_Buffer.h"
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "ogr_geometry.h"
#include "omp.h"

class MyGIS;
class Buffer : public QDialog
{
	Q_OBJECT

public:
	Buffer(MyGIS* mainWindow, QWidget *parent = nullptr);
	~Buffer();
	void addLayer(const QStringList& strlLayers);
	void createBuffer(const QString& strInput, const QString& strOutput, double dDistance);

public slots:
	void actionOpenFile();
	void actionYes();
	void actionNo();

private:
	Ui::BufferClass ui;
	MyGIS* mpMainWindow;
};
