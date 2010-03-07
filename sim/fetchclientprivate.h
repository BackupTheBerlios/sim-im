/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "simapi.h"

#include <QUrl>
#include <QHttp>

#include "fetch.h"

class CookieCash {
  typedef QMap< QString,QMap< QString,QString > > CCookieMap;
public:
  CookieCash(){};
  virtual ~CookieCash(){};

  void ProcessHeaders( const QString& headers );
  void CashCookie( const QString& header );
  QString GetCookie( QString domain );
  QString GetHeader( QString domain );
  void empty();

protected:
  CCookieMap mapCash;
};

class FetchClientPrivate : public QObject
{
    Q_OBJECT
public:
    FetchClientPrivate(FetchClient*);
    ~FetchClientPrivate();
protected:
    void fetch(const QString &url, const QString &headers, Buffer *postData, bool bRedirect);
    void stop();
    void clearCookies() { m_CookieCash.empty(); }

    FetchClient *m_client;
    void        fail();
    QHttp       *m_pHttp;
    QUrl        m_uri;
    QByteArray  m_postData;
    bool        m_bDone;
    CookieCash  m_CookieCash;

    unsigned	m_code;
    bool        m_bRedirect;
    unsigned    m_sendTime;
    unsigned    m_sendSize;
    unsigned	m_speed;
    enum State{
        SSLConnect,
        None,
        Header,
        Data,
        Done,
        Redirect
    };
    State       m_state;
    bool        m_bHTTPS;
    QHttpResponseHeader m_Response;

    friend class FetchClient;
protected slots:
    void authenticationRequired ( const QString & hostname, quint16 port, QAuthenticator * authenticator );
    void dataReadProgress ( int done, int total );
    void dataSendProgress ( int done, int total );
    void done ( bool error );
    void proxyAuthenticationRequired ( const QNetworkProxy & proxy, QAuthenticator * authenticator );
    void readyRead ( const QHttpResponseHeader & resp );
    void requestFinished ( int id, bool error );
    void requestStarted ( int id );
    void responseHeaderReceived ( const QHttpResponseHeader & resp );
    void sslErrors ( const QList<QSslError> & errors );
    void stateChanged ( int state );
};
