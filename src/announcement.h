#pragma once
#include <QString>
#include <QStringList>
#include <QRegularExpression>
// Audio-only projection shared by local SAPI and PrivateRadio. Originals stay intact.
inline QString frenchBelowHundred(int n){
 static const QStringList small{"zéro","un","deux","trois","quatre","cinq","six","sept","huit","neuf","dix","onze","douze","treize","quatorze","quinze","seize"};
 if(n<17)return small[n];
 if(n<20)return "dix-"+small[n-10];
 if(n<70){static const QStringList tens{"","","vingt","trente","quarante","cinquante","soixante"};return tens[n/10]+(n%10==0?QString():n%10==1?QString(" et un"):"-"+small[n%10]);}
 if(n<80)return "soixante"+(n==71?QString(" et onze"):"-"+frenchBelowHundred(n-60));
 return n==80?QString("quatre-vingts"):"quatre-vingt-"+frenchBelowHundred(n-80);
}
inline QString interventionSpeech(QString original){
 const QRegularExpression code(R"(\bP0[0-9]{2}\b)",QRegularExpression::CaseInsensitiveOption|QRegularExpression::UseUnicodePropertiesOption);
 auto first=code.match(original);
 QString text=first.hasMatch()?original.mid(first.capturedStart()).toLower():original;
 // Only information separators become commas; lexical hyphens remain intact.
 text.replace(QRegularExpression(R"(\s+[-–—]+\s+|\s*[–—]\s*)"),", ");
 auto replace=[&](const QRegularExpression& re,auto render){auto it=re.globalMatch(text);QString result;qsizetype pos=0;while(it.hasNext()){auto m=it.next();result+=text.mid(pos,m.capturedStart()-pos)+render(m);pos=m.capturedEnd();}text=result+text.mid(pos);};
 replace(code,[](const QRegularExpressionMatch& m){int n=m.captured().right(2).toInt();return "P zéro "+(n<10?QString("zéro "):QString())+frenchBelowHundred(n);});
 replace(QRegularExpression(R"(\b([01]?[0-9]|2[0-3])h([0-5][0-9])\b)",QRegularExpression::CaseInsensitiveOption),[](const QRegularExpressionMatch& m){int h=m.captured(1).toInt(),minutes=m.captured(2).toInt();return frenchBelowHundred(h)+(h==1?" heure":" heures")+(minutes?" "+frenchBelowHundred(minutes):QString());});
 replace(QRegularExpression(R"(\bN([0-9])\b)",QRegularExpression::CaseInsensitiveOption),[](const QRegularExpressionMatch& m){return "N "+frenchBelowHundred(m.captured(1).toInt());});
 text.replace(QRegularExpression(R"((\bN (?:zéro|un|deux|trois|quatre|cinq|six|sept|huit|neuf))\s+0\b)"),"\\1, zéro");
 // Ordinary quantities, addresses and technical identifiers are left to the French voice.
 text.replace(QRegularExpression(R"(\s+)")," ");return text.trimmed();
}
inline QString preparedAnnouncement(QString alias,QString message){auto body=interventionSpeech(std::move(message));alias=alias.trimmed();if(alias.isEmpty())return body;if(!alias.endsWith('.')&&!alias.endsWith('!')&&!alias.endsWith('?'))alias+='.';return alias+" "+body;}
