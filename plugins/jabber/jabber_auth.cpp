/***************************************************************************
                          jabber_auth.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#include "jabberclient.h"

#include <QTimer>
#include <QCryptographicHash>

using namespace SIM;

class AuthRequest : public JabberClient::ServerRequest
{
public:
    AuthRequest(JabberClient *client);
protected:
    bool m_bFail;
    virtual void element_start(const QString& el, const QXmlAttributes& attrs);
    virtual void element_end(const QString& el);
};

AuthRequest::AuthRequest(JabberClient *client)
        : JabberClient::ServerRequest(client, _SET, NULL, client->VHost())
{
    m_bFail = true;
}

void AuthRequest::element_end(const QString& el)
{
    if (el != "iq")
        return;
    if (m_bFail){
        QTimer::singleShot(0, m_client, SLOT(auth_failed()));
    }else{
        QTimer::singleShot(0, m_client, SLOT(auth_ok()));
    }
}

void AuthRequest::element_start(const QString& el, const QXmlAttributes& attrs)
{
    if (el == "iq"){
        QString value = attrs.value("type").toLower();
        if (value == "result")
            m_bFail = false;
    }
}

void JabberClient::auth_plain()
{
    AuthRequest *req = new AuthRequest(this);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:auth");
    QString username = data.owner.ID.str();
    username = getToken(username, '@');
    req->text_tag("username", username);
    req->text_tag("password", getPassword());
    req->text_tag("resource", data.owner.Resource.str());
    req->send();
    m_requests.push_back(req);
}

void JabberClient::auth_register()
{
    AuthRequest *req = new AuthRequest(this);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:register");
    QString username = data.owner.ID.str();
    username = getToken(username, '@');
    req->text_tag("username", username);
    req->text_tag("password", getPassword());
    req->send();
    m_requests.push_back(req);
}

void JabberClient::auth_digest()
{
    AuthRequest *req = new AuthRequest(this);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:auth");
    QString username = data.owner.ID.str();
    username = getToken(username, '@');
    req->text_tag("username", username);

    QString digest = m_id;
    digest += getPassword();
    QByteArray md = QCryptographicHash::hash(digest.toUtf8(), QCryptographicHash::Sha1);
    digest = QString::null;
    for (int i = 0; i < md.size(); i++){
        char b[3];
        sprintf(b, "%02x", md[i] & 0xFF);
        digest += b;
    }
    req->text_tag("digest", digest);
    req->text_tag("resource", data.owner.Resource.str());
    req->send();
    m_requests.push_back(req);
}
