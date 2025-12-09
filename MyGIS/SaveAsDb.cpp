#include "MyGIS.h"
#include "SaveAsDb.h"

SaveAsDb::SaveAsDb(MyGIS* mainWindow, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	mpMainWindow = mainWindow;

	connect(ui.pushButton, &QPushButton::clicked, this, &SaveAsDb::actionYes);
	connect(ui.pushButton_2, &QPushButton::clicked, this, &SaveAsDb::actionNo);
}

SaveAsDb::~SaveAsDb()
{}

void SaveAsDb::actionYes() {
    saveDb(ui.comboBox->currentText());
	this->close();
}

void SaveAsDb::actionNo() {
	this->close();
}

void SaveAsDb::addLayer(const QStringList& strlLayers) {
	ui.comboBox->addItems(strlLayers);
}

void SaveAsDb::saveDb(const QString& strInputPath) {
    // 打开矢量数据文件
    GDALDataset* poDataset = static_cast<GDALDataset*>(GDALOpenEx(strInputPath.toStdString().c_str(),
        GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
    if (poDataset == nullptr) {
        qDebug() << "Failed to open vector file: " << strInputPath;
        return;
    }

    // 获取文件名
    QFileInfo fileInfo(strInputPath);
    QString strFileName = fileInfo.baseName();  // 使用基础名称，去掉文件扩展名

    // 打开 PostgreSQL 数据库连接
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", "myConnectionName");
    db.setHostName("127.0.0.1");
    db.setDatabaseName("postgres");
    db.setPort(5432);
    db.setUserName("postgres");
    db.setPassword("6588312");
    if (!db.open()) {
        qDebug() << "Failed to connect to database: " << db.lastError().text();
        GDALClose(poDataset);
        return;
    }

    QSqlQuery query(db);

    // 获取矢量图层
    OGRLayer* poLayer = poDataset->GetLayer(0);
    if (poLayer == nullptr) {
        qDebug() << "Failed to get layer from vector file.";
        GDALClose(poDataset);
        db.close();
        return;
    }

    // 如果表存在，则删除它
    QString dropTableSQL = QString("DROP TABLE IF EXISTS %1;").arg(strFileName);
    if (!query.exec(dropTableSQL)) {
        qDebug() << "Failed to drop existing table: " << query.lastError().text();
        GDALClose(poDataset);
        db.close();
        return;
    }

    // 创建新表
    QString createTableSQL = QString("CREATE TABLE %1 (FeatureId SERIAL PRIMARY KEY").arg(strFileName);

    // 添加字段
    OGRFeatureDefn* poFeatureDefn = poLayer->GetLayerDefn();
    for (int i = 0; i < poFeatureDefn->GetFieldCount(); ++i) {
        OGRFieldDefn* poFieldDefn = poFeatureDefn->GetFieldDefn(i);
        createTableSQL += QString(", %1 VARCHAR").arg(poFieldDefn->GetNameRef());
    }

    // 添加几何字段（作为文本）
    createTableSQL += ", geom TEXT);";

    // 打印 SQL 语句以调试
    qDebug() << "Create Table SQL: " << createTableSQL;

    if (!query.exec(createTableSQL)) {
        qDebug() << "Failed to create table: " << query.lastError().text();
        GDALClose(poDataset);
        db.close();
        return;
    }

    // 插入数据
    OGRFeature* poFeature;
    poLayer->ResetReading();
    while ((poFeature = poLayer->GetNextFeature()) != nullptr) {
        QString insertSQL = QString("INSERT INTO %1 (").arg(strFileName);

        // 字段名称
        QStringList fieldNames;
        for (int i = 0; i < poFeatureDefn->GetFieldCount(); ++i) {
            fieldNames << poFeatureDefn->GetFieldDefn(i)->GetNameRef();
        }
        fieldNames << "geom";
        insertSQL += fieldNames.join(", ") + ") VALUES (";

        // 字段值
        QStringList fieldValues;
        for (int i = 0; i < poFeatureDefn->GetFieldCount(); ++i) {
            fieldValues << QString("'%1'").arg(poFeature->GetFieldAsString(i));
        }

        // 几何值
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        if (poGeometry) {
            char* geomWkt = nullptr;
            poGeometry->exportToWkt(&geomWkt);
            QString geomValue = QString::fromUtf8(geomWkt);
            CPLFree(geomWkt);
            fieldValues << "'" + geomValue + "'";
        }
        else {
            fieldValues << "NULL";
        }

        insertSQL += fieldValues.join(", ") + ");";

        if (!query.exec(insertSQL)) {
            qDebug() << "Failed to insert feature: " << query.lastError().text();
            GDALClose(poDataset);
            db.close();
            return;
        }

        OGRFeature::DestroyFeature(poFeature);
    }

    GDALClose(poDataset);
    db.close();
}
