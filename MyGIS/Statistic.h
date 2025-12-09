#pragma once

#include <QDialog>
#include <QTableWidget>

#include "ui_Statistic.h"

class MyGIS;
class Statistic : public QDialog
{
	Q_OBJECT

public:
	Statistic(MyGIS* mainWindow, QWidget *parent = nullptr);
	~Statistic();
	void addLayer(const QStringList& strlLayers);
	void analyzeVectorData(const QString& strFilePath, QTableWidget* pTable);

public slots:
	void actionYes();
	void actionNo();

private:
	Ui::StatisticClass ui;
	MyGIS* mpMainWindow;
};
