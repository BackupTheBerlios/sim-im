/***************************************************************************
                          fetch.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "simapi.h"

#include "fetch.h"
#include "fetchclientprivate.h"
#include "socket/socket.h"
#include "socket/socketfactory.h"
#include "misc.h"
#include "log.h"

using namespace SIM;

void CookieCash::ProcessHeaders( const QString& headers ) {
    QString head = headers;
    while (!head.isEmpty()){
        QString header = getToken(head, '\n');
        QString key = getToken(header, ':');
        header = header.trimmed();
        if( key == "Set-Cookie" ) {
            CashCookie( header );
        }
    }
}

void CookieCash::CashCookie( const QString& header ) {
    QString domain;
    QString expares;
    QString path;
    QMap<QString,QString> values;
    QString head = header;
    while (!head.isEmpty()){
        QString cookie = getToken(head, ';');
        QString name = getToken(cookie, '=');
        name=name.trimmed();
        QString value = cookie.trimmed();
        if( name == "domain" ) {
            domain=value;
        }
        else if( name == "expires" ) {
            expares=value;
        }
        else if( name == "path" ) {
            path=value;
        }
        else {
            values[name]=value;
        }
    }
    if( values.count() > 0 ) {
        CCookieMap::iterator it = mapCash.find(domain);
        if( it == mapCash.end() ) {
            mapCash[domain]=values;
        }
        else {
            QMap<QString,QString>::iterator mit;
            for( mit = values.begin() ; mit != values.end() ; mit++ ) {
                if(mit.value()=="deleted"){
                    it.value().remove(mit.key());
                }
                else{
                    it.value()[mit.key()]=mit.value();
                }
            }
        }
    }
}

QString CookieCash::GetCookie( QString domain ){
    QString cookie;

    foreach( QString dom, mapCash.keys() ) {
        if(0!=domain.contains(dom)) {
            foreach( QString key, mapCash.value(dom).keys() ) {
                cookie += key + QString("=") + mapCash.value(dom).value(key) + ";";
            }
        }
    }

    return cookie;
}

QString CookieCash::GetHeader( QString /* domain */) {
    QString cookie = GetCookie( "pda.vkontakte.ru" );
    if( !cookie.isEmpty() ) {
        cookie = QString("Cookie: ") + cookie + QString( "\r\n" );
    }
    return cookie;
}

void CookieCash::empty() {
        mapCash.clear();
}

const unsigned UNKNOWN_SIZE = (unsigned)(-1);

FetchClient::FetchClient()
  : p( NULL )
{
    p = new FetchClientPrivate(this);
}

FetchClient::~FetchClient()
{
    if ( NULL != p ){
        delete p;
        p = NULL;
    }
}

void FetchClient::set_speed(unsigned speed)
{
    p->m_speed = speed;
}

void FetchClient::fetch(const QString &url, const QString &headers, Buffer *postData, bool bRedirect)
{
    getSocketFactory()->checkState();
    p->fetch(url, headers, postData, bRedirect);
}

void FetchClient::stop()
{
    p->stop();
}

bool FetchClient::isDone()
{
    return p->m_bDone;
}

void FetchClient::clearCookies()
{
    p->clearCookies();
}

FetchClientPrivate::FetchClientPrivate(FetchClient *client)
  : m_client( client )
  , m_pHttp( NULL )
  , m_bDone( true )
  , m_code( 0 )
  , m_bRedirect( true )
  , m_state( None )
{
}

void FetchClientPrivate::fetch(const QString &url, const QString &headers, Buffer *postData, bool bRedirect)
{
    stop();
    m_uri = url;
    m_bRedirect = bRedirect;
    QHttp::ConnectionMode mode = QHttp::ConnectionModeHttp;
    if(m_uri.scheme() == "https")
        mode = QHttp::ConnectionModeHttps;
    else if(m_uri.scheme() != "http")
        return;

    m_pHttp = new QHttp(m_uri.host());
//    connect( m_pHttp, SIGNAL(authenticationRequired(QString&,quint16,QAuthenticator*)),SLOT(authenticationRequired(QString&,quint16,QAuthenticator*)));
    connect( m_pHttp, SIGNAL(dataReadProgress(int,int)),SLOT(dataReadProgress(int,int)));
    connect( m_pHttp, SIGNAL(dataSendProgress(int,int)),SLOT(dataSendProgress(int,int)));
    connect( m_pHttp, SIGNAL(done(bool)),SLOT(done(bool)));
    connect( m_pHttp, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)),SLOT(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));
    connect( m_pHttp, SIGNAL(readyRead(const QHttpResponseHeader&)),SLOT(readyRead(const QHttpResponseHeader&)));
    connect( m_pHttp, SIGNAL(requestFinished(int, bool)),SLOT(requestFinished(int, bool)));
    connect( m_pHttp, SIGNAL(requestStarted(int)),SLOT(requestStarted(int)));
    connect( m_pHttp, SIGNAL(responseHeaderReceived(const QHttpResponseHeader&)),SLOT(responseHeaderReceived(const QHttpResponseHeader&)));
    connect( m_pHttp, SIGNAL(sslErrors(const QList<QSslError>&)),SLOT(sslErrors(const QList<QSslError>&)));
    connect( m_pHttp, SIGNAL(stateChanged(int)),SLOT(stateChanged(int)));

    m_postData.clear();

    QString sMethod = "GET";
    if( NULL != postData ) {
        m_postData = *postData;
        sMethod = "POST";
    }

    log( L_DEBUG, "HTTP: Started %s request for %s", qPrintable( sMethod ), qPrintable( url ) );

    QString sHost = m_uri.host();
    QString sPath = m_uri.encodedPath();
    if( sPath.isEmpty() ) {
        sPath = "/";
    }
    QString sQuery = m_uri.encodedQuery();
    if( !sQuery.isEmpty() ) {
        sPath = sPath + "?" + sQuery;
    }

    QString sCookies = m_CookieCash.GetHeader( "pda.vkontakte.ru" );
    QString sHead = headers;
    if( !sCookies.isEmpty() ) {
        sHead += "\n" + sCookies;
    }

    QHttpRequestHeader header(sMethod, sPath);
    header.setValue("Host", sHost);
    m_pHttp->setHost(sHost);
    while( !sHead.isEmpty() ) {
        QString val = getToken(sHead, '\n');
        QString key = getToken(val, ':');
        val = val.trimmed();
        header.addValue( key, val.trimmed() );
    }

    m_bDone = false;

    if( NULL != postData ) {
        m_postData = *postData;
        m_pHttp->request(header,m_postData);
    }
    else {
        m_pHttp->request(header);
    }
}

FetchClientPrivate::~FetchClientPrivate()
{
    m_client = NULL;
    stop();
}

void FetchClientPrivate::stop()
{
    if( NULL != m_pHttp ) {
        disconnect(m_pHttp, 0, 0, 0);
        m_pHttp->abort();
        delete m_pHttp;
        m_pHttp = NULL;
    }
}

bool FetchClient::crackUrl(const QString &_url, QString &protocol, QString &host, unsigned short &port, QString &user, QString &pass, QString &uri, QString &extra)
{
    QUrl url(_url);
    if(!url.isValid())
        return false;
    protocol = url.scheme();
    host = url.host();
    user = url.userName();
    pass = url.password();
    if (protocol == "https")
        port = 443;
    else
        port = 80;
    port = url.port(port);
    uri = url.path();
    extra = url.fragment();
    return true;
}

QByteArray basic_auth(const QString &user, const QString &pass)
{
    QString auth = user + ':' + pass;
    return auth.toLocal8Bit().toBase64();
}

unsigned FetchClient::post_size()
{
    return p->m_postData.size();
}

bool get_connection_state(bool&)
{
    return false;
}

QString get_user_agent()
{
    QString user_agent = "Mozilla/4.0 (" PACKAGE "/" VERSION " ";
    user_agent += get_os_version();
    user_agent += ')';
    return user_agent;
}

const char *FetchClient::read_data(char * /*buf */, unsigned& /*size*/){ return NULL; }
bool     FetchClient::write_data(const char * /*buf*/, unsigned /*size*/){ return false; }

void FetchClientPrivate::authenticationRequired ( const QString & /*hostname*/, quint16 /*port*/, QAuthenticator * /*authenticator*/ ) {
}

void FetchClientPrivate::dataReadProgress ( int /*done*/, int /*total*/ ) {
}

void FetchClientPrivate::dataSendProgress( int/* done*/, int /*total*/ ) {
}

void FetchClientPrivate::done(bool error ) {
    if( error ) {
        log( L_DEBUG, "HTTP: Request done with error!" );
        if( NULL != m_client ) {
            Buffer buf = QByteArray();
            QString str;
            m_client->done( 0, buf, str );
        }
        m_bDone = true;
        return;
    }

    log( L_DEBUG, "HTTP: Request done without error." );

    QString sHeaders;
    QList<QPair<QString, QString> > vals = m_Response.values();
    QPair<QString, QString> pair;
    foreach( pair, vals ) {
        sHeaders += pair.first;
        sHeaders += ": ";
        sHeaders += pair.second;
        sHeaders += "\n";
    }
    m_CookieCash.ProcessHeaders( sHeaders );

    m_code = m_Response.statusCode();
    if( m_bRedirect && ( 300 <= m_code ) && ( 307 >= m_code )) {
        QUrl urlLocation = m_Response.value( "Location" );
        if( urlLocation.host().isEmpty() ) {
            urlLocation.setHost( m_uri.host() );
        }
        if( urlLocation.scheme().isEmpty() ) {
            urlLocation.setScheme( m_uri.scheme() );
        }
        Buffer buf = m_postData;
        m_state = Redirect;
        fetch(urlLocation.toString(),QString(),&buf,true);
        return;
    }
    if( NULL != m_client ) {
        Buffer buf = m_pHttp->readAll();
        m_client->done( m_code, buf, sHeaders );
    }
    m_bDone = true;
}

void FetchClientPrivate::proxyAuthenticationRequired( const QNetworkProxy & /*proxy*/, QAuthenticator * /*authenticator*/ ) {
}

void FetchClientPrivate::readyRead( const QHttpResponseHeader &/*resp*/ ) {
}

void FetchClientPrivate::requestFinished( int /*id*/, bool /*error*/ ) {
}

void FetchClientPrivate::requestStarted( int /*id */) {
}

void FetchClientPrivate::responseHeaderReceived( const QHttpResponseHeader & resp ) {
    m_Response = resp;
}

void FetchClientPrivate::sslErrors( const QList<QSslError> & /*errors*/ ) {
}

void FetchClientPrivate::stateChanged( int /*state*/ ) {
}

