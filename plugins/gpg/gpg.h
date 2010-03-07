/***************************************************************************
                          gpg.h  -  description
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

#ifndef _GPG_H
#define _GPG_H

#include "cfg.h"
#include "event.h"
#include "plugins.h"
#include "propertyhub.h"

#include <QObject>
#include <QString>
#include <QList>
#include <QByteArray>

const unsigned long MessageGPGKey       = 0x5000;
const unsigned long MessageGPGUse       = 0x5001;

struct GpgUserData
{
    SIM::Data   Key;
    SIM::Data   Use;
};

class QProcess;

struct DecryptMsg
{
    SIM::Message *msg;
    QProcess    *process;
    QString     infile;
    QString     outfile;
    unsigned    contact;
    QString     passphrase;
    QString     key;
};

struct KeyMsg
{
    QString         key;
    SIM::Message   *msg;
};

class GpgPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
	Q_OBJECT
public:
    GpgPlugin(unsigned, Buffer*);
    virtual ~GpgPlugin();

    QString GPG();
    void reset();
    static GpgPlugin *plugin;
    QList<KeyMsg>	 m_sendKeys;
    unsigned long user_data_id;
    QString getHomeDir();

    void setPropertyHub(SIM::PropertyHubPtr hub);
    SIM::PropertyHubPtr propertyHub();
    QVariant value(const QString& key);
    void setValue(const QString& key, const QVariant& v);

protected slots:
    void decryptReady();
    void importReady();
    void publicReady();
    void clear();
    void passphraseFinished();
    void passphraseApply(const QString&);
protected:
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual QByteArray getConfig();
    virtual bool processEvent(SIM::Event *e);
    void registerMessage();
    void unregisterMessage();
    void askPassphrase();
    bool decode(SIM::Message *msg, const QString &pass, const QString &key);
    bool m_bMessage;
    QList<DecryptMsg> m_decrypt;
    QList<DecryptMsg> m_import;
    QList<DecryptMsg> m_public;
    QList<DecryptMsg> m_wait;
    class PassphraseDlg     *m_passphraseDlg;
private:
    SIM::PropertyHubPtr m_propertyHub;
};

class MsgEdit;

class MsgGPGKey : public QObject, public SIM::EventReceiver
{
    Q_OBJECT
public:
    MsgGPGKey(MsgEdit *parent, SIM::Message *msg);
    ~MsgGPGKey();
protected slots:
    void init();
    void exportReady();
protected:
    virtual bool processEvent(SIM::Event *e);
    QString     m_client;
    QString     m_key;
    MsgEdit     *m_edit;
    QProcess	*m_process;
};

#endif

