#include "window.h"
#include <QApplication>
#include <QMessageBox>
int main(int argc,char** argv){QApplication app(argc,argv);app.setOrganizationName("POCSAG-SDR-Pro");app.setApplicationName("POCSAG-SDR-Pro");try{Window w;w.show();return app.exec();}catch(const std::exception& e){QMessageBox::critical(nullptr,"Démarrage impossible",QString::fromUtf8(e.what()));return 1;}}
