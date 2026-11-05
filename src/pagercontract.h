#pragma once
#include <QJsonObject>
#include <QDateTime>
// Offline contract only: no network transport, endpoint or credentials.
inline bool pagerEligible(bool generalEnabled,bool registered,bool perCapcode){return generalEnabled&&registered&&perCapcode;}
inline QJsonObject pagerEnvelope(QString eventId,quint32 capcode,QString alias,QDateTime utc,quint32 hz,QString source,QString text){
 return {{"schema",1},{"message_id",eventId},{"capcode",QString("%1").arg(capcode,7,10,QChar('0'))},{"alias",alias},{"timestamp_utc",utc.toUTC().toString(Qt::ISODateWithMs)},{"frequency_hz",double(hz)},{"source",source},{"text",text},{"utterance",alias.isEmpty()?text:alias+". "+text}};
}
