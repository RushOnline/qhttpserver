#include "bodydata.h"

#include <QCoreApplication>
#include <QRegExp>
#include <QStringList>
#include <QDebug>

#include <qhttpserver.h>
#include <qhttprequest.h>
#include <qhttpresponse.h>

/// BodyData

BodyData::BodyData()
{
    QHttpServer *server = new QHttpServer(this);
    connect(server, SIGNAL(newRequest(QHttpRequest*, QHttpResponse*)),
        this, SLOT(handleRequest(QHttpRequest*, QHttpResponse*)));
        
    server->listen(QHostAddress::Any, 8081);
}

void BodyData::handleRequest(QHttpRequest *req, QHttpResponse *resp)
{
    new Responder(req, resp);
}

/// Responder

Responder::Responder(QHttpRequest *req, QHttpResponse *resp)
    : m_req(req)
    , m_resp(resp)
{
    QRegExp exp("^/user/([a-z]+$)");
    if (exp.indexIn(req->path()) == -1)
    {
        resp->writeHead(403);
        resp->end("You aren't allowed here!");
        /// @todo There should be a way to tell request to stop streaming data
        return;
    }

    resp->setHeader("Content-Type", "text/html");
    resp->writeHead(200);
    
    QString name = exp.capturedTexts()[1];
    QString bodyStart = tr("<html><head><title>BodyData App</title></head><body><h1>Hello %1!</h1><p>").arg(name);
    resp->write(bodyStart);

    connect(req, SIGNAL(data(const QByteArray&)), this, SLOT(accumulate(const QByteArray&)));
    connect(req, SIGNAL(end()), this, SLOT(reply()));
    connect(m_resp, SIGNAL(done()), this, SLOT(deleteLater()));
}

Responder::~Responder()
{
}

void Responder::accumulate(const QByteArray &data)
{
    m_resp->writeByteArray(data);
}

void Responder::reply()
{
    m_resp->end("</p></body></html>");
}

/// main

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    BodyData bodydata;
    app.exec();
}
