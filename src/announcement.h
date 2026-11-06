#pragma once
#include <QString>
#include <QStringList>
#include <QRegularExpression>
// Audio-only projection. Never write this value into the original message fields.
inline QString interventionSpeech(QString original){
 const QRegularExpression code(R"(\bP0[0-9]{2}\b)",QRegularExpression::CaseInsensitiveOption|QRegularExpression::UseUnicodePropertiesOption);
 auto match=code.match(original);if(!match.hasMatch())return original;
 QString text=original.mid(match.capturedStart()).toLower();
 text.replace(QRegularExpression(R"(\b(n[0-9]+)\s+(?=[0-9]))"),"\\1, ");
 text.replace(QRegularExpression(R"(([\p{L}])(?=[0-9]))"),"\\1 ");
 static const QStringList digits{"zéro","un","deux","trois","quatre","cinq","six","sept","huit","neuf"};
 QRegularExpression numbers("[0-9]+");auto it=numbers.globalMatch(text);QString spoken;qsizetype position=0;while(it.hasNext()){auto m=it.next();spoken+=text.mid(position,m.capturedStart()-position);QStringList words;for(auto c:m.captured())words<<digits[c.digitValue()];spoken+=words.join(' ');position=m.capturedEnd();}spoken+=text.mid(position);
 spoken.replace(QRegularExpression(R"(\s*[-–—]+\s*)"),", ");spoken.replace(QRegularExpression(R"(\bp\s)"),"P ");spoken.replace(QRegularExpression(R"(\bn\s)"),"N ");spoken.replace(QRegularExpression(R"(\s+)")," ");return spoken.trimmed();
}
inline QString preparedAnnouncement(QString alias,QString message){auto body=interventionSpeech(std::move(message));alias=alias.trimmed();return alias.isEmpty()?body:alias+". "+body;}
