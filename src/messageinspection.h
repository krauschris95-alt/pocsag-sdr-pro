#pragma once
#include <QSqlQuery>
#include <QString>
#include "payloadanalysis.h"
inline QString messageInspection(const QSqlQuery& q){
 const bool noRaw=q.value(1).toString().isEmpty()&&q.value(11).toString().isEmpty();
 const bool legacy=noRaw&&q.value(12).toString().isEmpty();
 const QString unavailable=legacy?"Données non disponibles pour cet ancien enregistrement":"Source sans données radio brutes : texte fourni directement";
 auto field=[&](int index,QString empty){auto s=q.value(index).toString();return s.isEmpty()?empty:s;};
 QString missing=noRaw?unavailable:"Aucun caractère dans cette interprétation (voir les bits conservés)";
 QString details="Message reçu (original conservé) :\n"+field(0,"Message sans texte")+"\nDate : "+field(14,"Date non disponible")+"\nType affiché : "+field(13,"Type non enregistré");
 auto assessment=pocsag::assessAlpha(q.value(0).toString().toStdString());
 if(assessment.base64){
  details+="\nAnalyse du texte (sans modifier l'original) : "+QString::fromStdString(assessment.description)+". Cette analyse seule n'identifie pas le format radio.";
  const auto binary=pocsag::inspectBase64(q.value(0).toString().toStdString());
  if(binary.recognized){
   details+="\n\nDÉCODAGE BASE64 — ANALYSE LOCALE, SANS DÉCHIFFREMENT";
   details+="\nLongueur décodée : "+QString::number(qulonglong(binary.byteCount))+" octets";
   details+="\nIdentification indicative : "+QString::fromStdString(binary.signature);
   details+="\nEntropie indicative : "+QString::number(binary.entropyBitsPerByte,'f',2)+" bits/octet (ne prouve pas un chiffrement)";
   details+="\nOctets en hexadécimal (aperçu) :\n"+QString::fromStdString(binary.hexPreview);
   details+="\nAperçu ASCII (les octets non imprimables sont remplacés par des points) :\n"+QString::fromStdString(binary.asciiPreview);
   if(binary.truncated)details+="\nAperçu limité aux premiers octets ; le message original reste conservé intégralement.";
   details+="\nBase64 est un encodage, pas une preuve de chiffrement. Algorithme AES/IDEA, réseau émetteur et format applicatif non identifiés. Aucun déchiffrement sans spécification et clé autorisée.";
  }
 }
 if(noRaw)details+="\n\n"+unavailable+". L'absence de métadonnées ne prouve pas une erreur de réception ; la provenance historique ne peut pas être reconstruite.";
 details+="\n\nALPHA :\n"+field(2,missing)+"\n\nNUMERIC :\n"+field(3,missing)+"\n\nDiagnostic : "+field(4,noRaw?unavailable:"État non renseigné par la source")+"\nPolitique : "+field(5,"Politique non enregistrée à l'époque de la réception")+"\nFin : "+field(6,noRaw?unavailable:"Motif non renseigné")+" ; complet="+(legacy?QString("non documenté (ancien enregistrement)"):field(7,"non renseigné"))+"\nFonction : "+field(8,"non enregistrée")+" ; bits corrigés="+field(9,"non enregistrés")+"\nÉvénement : "+field(10,"Identifiant non enregistré");
 details+="\n\nBits utiles après correction BCH (ordre reçu) :\n"+field(1,noRaw?unavailable:"Aucun bit utile : adresse seule (TONE)")+"\n\nMots radio avant correction (hex ; adresse, données, limite) :\n"+field(11,unavailable);
 return details;
}
