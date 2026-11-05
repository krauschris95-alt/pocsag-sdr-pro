#pragma once
#include <QApplication>
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <exception>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstdio>
inline wchar_t fatalPath[32768]{};
inline LONG WINAPI logWindowsFault(EXCEPTION_POINTERS* p){
 HANDLE f=CreateFileW(fatalPath,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
 if(f!=INVALID_HANDLE_VALUE){char text[128];int n=std::snprintf(text,sizeof(text),"Windows unhandled exception 0x%08lX at %p\r\n",p->ExceptionRecord->ExceptionCode,p->ExceptionRecord->ExceptionAddress);DWORD written;WriteFile(f,text,DWORD(n),&written,nullptr);CloseHandle(f);}return EXCEPTION_CONTINUE_SEARCH;
}
#endif
inline void logException(QString text){QString dir=QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);QDir().mkpath(dir);QFile f(dir+"/exceptions.log");if(f.size()>1024*1024){QFile::remove(f.fileName()+".1");f.rename(f.fileName()+".1");f.setFileName(dir+"/exceptions.log");}if(f.open(QIODevice::WriteOnly|QIODevice::Append))f.write((QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)+" "+text+"\r\n").toUtf8());}
inline void installExceptionLog(){
 QString dir=QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);QDir().mkpath(dir);
#ifdef Q_OS_WIN
 auto path=QDir::toNativeSeparators(dir+"/exceptions.log");path.toWCharArray(fatalPath);SetUnhandledExceptionFilter(logWindowsFault);
#endif
 std::set_terminate([]{logException("Unhandled C++ termination");std::abort();});
}
class SafeApplication:public QApplication {
public:using QApplication::QApplication;
 bool notify(QObject* object,QEvent* event)override{try{return QApplication::notify(object,event);}catch(const std::exception& e){logException(QString::fromUtf8(e.what()));QMessageBox::warning(nullptr,"Erreur de l'application","L'opération a échoué. Consultez exceptions.log ; les données déjà enregistrées restent disponibles.");}catch(...){logException("Unknown event exception");QMessageBox::warning(nullptr,"Erreur de l'application","Une erreur inattendue a été interceptée. Consultez exceptions.log.");}return false;}
};
