#include "window.h"
#include <QApplication>
#include <QMessageBox>
#include <QTimer>
int main(int argc,char** argv){QApplication app(argc,argv);app.setOrganizationName("POCSAG-SDR-Pro");app.setApplicationName("POCSAG-SDR-Pro");try{Window w;w.show();auto args=app.arguments();int shot=args.indexOf("--screenshot");if(shot>=0&&shot+1<args.size())QTimer::singleShot(500,&w,[&]{bool ok=w.grab().save(args[shot+1]);app.exit(ok?0:2);});return app.exec();}catch(const std::exception& e){QMessageBox::critical(nullptr,"Démarrage impossible",QString::fromUtf8(e.what()));return 1;}}
