#pragma once

#ifndef IVECTORSTORAGE_H
#define IVECTORSTORAGE_H

#include "MyLayer.h"  
#include <QPen>
#include <QBrush>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>

class vectorStorage {
public:
	virtual ~vectorStorage() {}

	// ´æ´¢Ê¸Á¿Êý¾Ý
	virtual void storeVectorData(const MyVectorLayer& layer) = 0;

};

#endif 
