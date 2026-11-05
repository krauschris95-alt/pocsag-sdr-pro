#pragma once
#include <QtCore>
#include <QFileInfo>
inline QString normalizedTxtPath(QString value){return QDir::fromNativeSeparators(value.trimmed());}
inline bool absoluteTxtPath(const QString& value){
#ifdef Q_OS_WIN
 return QRegularExpression("^(?:[A-Za-z]:/|//[^/]+/[^/]+(?:/|$))").match(value).hasMatch();
#else
 return QDir::isAbsolutePath(value);
#endif
}
inline QString txtPathKey(QString value){value=QDir::cleanPath(QFileInfo(normalizedTxtPath(value)).absoluteFilePath());
#ifdef Q_OS_WIN
 value=value.toCaseFolded();
#endif
 return value;
}
inline QString validateTxtFile(QString value){value=normalizedTxtPath(value);if(!absoluteTxtPath(value)||QFileInfo(value).fileName().isEmpty()||QFileInfo(value).isDir())return "Choisissez le chemin absolu d’un fichier, existant ou nouveau.";
 QString tail=value;if(QRegularExpression("^[A-Za-z]:").match(tail).hasMatch())tail=tail.mid(2);
 if(tail.contains(QRegularExpression(R"([<>|"*?:\x00-\x1f])")))return "Caractère interdit dans le chemin.";
 for(auto part:tail.split('/',Qt::SkipEmptyParts))if(part!="."&&part!=".."&&(part.endsWith('.')||part.endsWith(' ')||QRegularExpression(R"(^(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])(?:\.|$))",QRegularExpression::CaseInsensitiveOption).match(part).hasMatch()))return "Nom réservé ou nom Windows invalide.";
 auto directory=QFileInfo(value).absolutePath();if(!QDir().mkpath(directory))return "Dossier inaccessible : vérifiez le disque, le partage et les autorisations Windows.";
 QTemporaryFile probe(QDir(directory).filePath(".pocsag-write-test-XXXXXX"));if(!probe.open()||probe.write("POCSAG")!=6||!probe.flush())return probe.errorString();
 QFile file(value);if(!file.open(QIODevice::WriteOnly|QIODevice::Append)||!file.flush())return file.errorString();return {};
}
