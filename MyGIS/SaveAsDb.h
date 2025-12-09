#pragma once

#include <QDialog>
#include "ui_SaveAsDb.h"

class MyGIS;
class SaveAsDb : public QDialog
{
	Q_OBJECT

public:
	SaveAsDb(MyGIS* mainWindow, QWidget *parent = nullptr);
	~SaveAsDb();
	void addLayer(const QStringList& strlLayers);
	void saveDb(const QString& strInputPath);

public slots:
	void actionYes();
	void actionNo();

private:
	Ui::SaveAsDbClass ui;
	MyGIS* mpMainWindow;
};
