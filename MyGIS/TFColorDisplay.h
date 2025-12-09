#pragma once

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QPixmap>

#include "ui_TFColorDisplay.h"
#include "omp.h"
#include "gdal_priv.h"

class MyGIS;
class TFColorDisplay : public QDialog
{
	Q_OBJECT

public:
	TFColorDisplay(MyGIS* mainWindow, QWidget *parent = nullptr);
	~TFColorDisplay();
	void addLayer(const QStringList& strlLayers);
	void displayCompositeImage(const QString& strFilePath, int nRedBand, int nGreenBand, int nBlueBand);
	void initialiseSpinBox(const QString& strFilePath);

public slots:
	void actionYes();
	void actionNo();
	void actionReread();

private:
	Ui::TFColorDisplayClass ui;
	MyGIS* mpMainWindow;
};
