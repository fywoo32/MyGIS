#pragma once

#include <QDialog>
#include "ui_SaveAsCsv.h"

class MyGIS;
class SaveAsCsv : public QDialog
{
	Q_OBJECT

public:
	SaveAsCsv(MyGIS* mainWindow, QWidget *parent = nullptr);
	~SaveAsCsv();
	void addLayer(const QStringList& strlLayers);
	void saveCSV(const QString& strInputPath, const QString& strOutputPath);

public slots:
	void actionOpenFile();
	void actionYes();
	void actionNo();

private:
	Ui::SaveAsCsvClass ui;
	MyGIS* mpMainWindow;
};
