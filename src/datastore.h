#pragma once
#include <QtCore>
struct StoreMigration {bool ok=false;QString error,backup,staging;};
QString resolveDataStore(QString defaultDirectory);
StoreMigration copyDataStore(QString source,QString destination,QString locatorFile);
bool activateDataStore(QString directory,QString previous,QString locatorFile,QString* error);
