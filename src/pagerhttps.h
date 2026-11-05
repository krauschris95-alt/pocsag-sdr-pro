#pragma once
#include "pagerworker.h"
#include <QUrl>
#include <QSslCertificate>
// Explicit LAB adapter; never constructed by the application before server integration.
class HttpsPagerTransport final:public PagerTransport{
public:HttpsPagerTransport(QUrl endpoint,QString bearer,QList<QSslCertificate> trustedLabCa={}):endpoint_(std::move(endpoint)),bearer_(std::move(bearer)),ca_(std::move(trustedLabCa)){}
 bool encryptedAndAuthenticated()const override;
 PagerAck deliver(const QJsonObject&,const QByteArray&)override;
private:QUrl endpoint_;QString bearer_;QList<QSslCertificate> ca_;
};
