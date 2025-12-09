#pragma once

#include <QStandardItem>

#include "MyLayer.h"


class MyItem : public QStandardItem {
public:
	MyItem() : QStandardItem() {}
	MyItem(const QString& text) : QStandardItem(text) {}
	MyItem(MyVectorLayer* pLayer, const QString& text) : QStandardItem(text) {
		mpVectorLayer = pLayer;
		mpRasterLayer = nullptr;
	}
	MyItem(MyRasterLayer* pLayer, const QString& text) : QStandardItem(text) {
		mpRasterLayer = pLayer;
		mpVectorLayer = nullptr;
	}

	~MyItem(){
		if (mpVectorLayer) {
			delete mpVectorLayer;
		}
		if (mpRasterLayer) {
			delete mpRasterLayer;
		}
		mpVectorLayer = nullptr;
		mpRasterLayer = nullptr;
	}

	MyVectorLayer* getVectorLayer() {
		return mpVectorLayer;
	}

	MyRasterLayer* getRasterLayer() {
		return mpRasterLayer;
	}

	void setVectorLayer(MyVectorLayer* pVectorLayer) {
		mpVectorLayer = pVectorLayer;
	}

	void setRasterLayer(MyRasterLayer* pRasterLayer) {
		mpRasterLayer = pRasterLayer;
	}

private:
	MyVectorLayer* mpVectorLayer;
	MyRasterLayer* mpRasterLayer;
};
