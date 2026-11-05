#include "pagerhttps.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include "pagercontract.h"
bool HttpsPagerTransport::encryptedAndAuthenticated()const{return endpoint_.scheme()=="https"&&!endpoint_.host().isEmpty()&&endpoint_.userInfo().isEmpty()&&!bearer_.isEmpty()&&!bearer_.contains('\r')&&!bearer_.contains('\n')&&QSslSocket::supportsSsl();}
PagerAck HttpsPagerTransport::deliver(const QJsonObject& event,const QByteArray& audio){if(!encryptedAndAuthenticated())return {false,{},"Transport HTTPS authentifié requis"};QNetworkAccessManager manager;QNetworkRequest request(endpoint_);request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");request.setRawHeader("Authorization",("Bearer "+bearer_).toUtf8());request.setRawHeader("Idempotency-Key",event["message_id"].toString().toUtf8());request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::ManualRedirectPolicy);request.setTransferTimeout(15000);auto ssl=QSslConfiguration::defaultConfiguration();ssl.setPeerVerifyMode(QSslSocket::VerifyPeer);ssl.setProtocol(QSsl::TlsV1_2OrLater);if(!ca_.isEmpty())ssl.setCaCertificates(ca_);request.setSslConfiguration(ssl);auto reply=manager.post(request,QJsonDocument(QJsonObject{{"event",pagerRemoteEnvelope(event)},{"audio_base64",QString::fromLatin1(audio.toBase64())}}).toJson(QJsonDocument::Compact));QEventLoop loop;QTimer timeout;timeout.setSingleShot(true);QObject::connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);QObject::connect(&timeout,&QTimer::timeout,&loop,[&]{reply->abort();loop.quit();});timeout.start(16000);loop.exec();auto status=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();auto error=reply->error();auto body=reply->readAll();delete reply;if(error!=QNetworkReply::NoError||status<200||status>=300)return {false,{},"TLS, autorisation ou accusé HTTP refusé"};auto ack=QJsonDocument::fromJson(body).object();return {ack["accepted"].toBool(),ack["message_id"].toString(),{}};}
