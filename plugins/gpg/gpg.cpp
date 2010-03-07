/***************************************************************************
                          gpg.cpp  -  description
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

#include "log.h"
#include "msgedit.h"
#include "userwnd.h"
#include "passphrase.h"

#include "simgui/textshow.h"
#include "simgui/ballonmsg.h"
#include "contacts/contact.h"
#include "contacts/client.h"

#include "profile.h"
#include "profilemanager.h"

#include "gpg.h"
#include "gpgcfg.h"
#include "gpguser.h"

#include <QTimer>
#include <QFile>
#include <QFileInfo>
#include <QRegExp>
#include <QProcess>
#include <QApplication> //for Linux: qApp->processEvents();
#include <QList>
#include <QByteArray>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

using namespace SIM;

#ifndef WIN32
static QString GPGpath;
#endif

Plugin *createGpgPlugin(unsigned base, bool, Buffer *cfg)
{
#ifndef WIN32
    if (GPGpath.isEmpty())
        return NULL;
#endif
    Plugin *plugin = new GpgPlugin(base, cfg);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("GPG"),
        I18N_NOOP("Plugin adds GnuPG encryption/decryption support for messages"),
        VERSION,
        createGpgPlugin, PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
#ifndef WIN32
    QString path;
    const char *p = getenv("PATH");
    if (p)
        path = QFile::decodeName(p);
    while (!path.isEmpty()){
        QString p = getToken(path, ':');
        p += "/gpg";
        QFile f(p);
        QFileInfo fi(f);
        if (fi.isExecutable()){
            GPGpath = p;
            break;
        }
    }
    if (GPGpath.isEmpty())
        info.description = I18N_NOOP("Plugin adds GnuPG encryption/decryption support for messages\n"
                                     "GPG not found in PATH");
#endif
    return &info;
}

static DataDef gpgUserData[] =
    {
        { "Key", DATA_STRING, 1, 0 },
        { "Use", DATA_BOOL, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

GpgPlugin *GpgPlugin::plugin = NULL;

GpgPlugin::GpgPlugin(unsigned base, Buffer *cfg)
    : QObject(), Plugin(base)
    , EventReceiver(HighestPriority - 0x100) //??? Fixmee!!
    , m_bMessage(false)
    , m_passphraseDlg(NULL)
{
    m_propertyHub = SIM::PropertyHub::create("gpg");
    user_data_id = getContacts()->registerUserData(info.title, gpgUserData);
    reset();
    plugin = this;
}

GpgPlugin::~GpgPlugin()
{
    delete m_passphraseDlg;
    unregisterMessage();
    QList<DecryptMsg>::ConstIterator it;
    for (it = m_decrypt.constBegin(); it != m_decrypt.constEnd(); ++it){
        delete it->msg;
        delete it->process;
    }
    for (it = m_import.constBegin(); it != m_import.constEnd(); ++it){
        delete it->msg;
        delete it->process;
    }
    for (it = m_public.constBegin(); it != m_public.constEnd(); ++it)
        delete it->process;
    for (it = m_wait.constBegin(); it != m_wait.constEnd(); ++it)
        delete it->msg;
    getContacts()->unregisterUserData(user_data_id);
}

QString GpgPlugin::GPG()
{
#ifdef WIN32
    return value("GPG").toString();
#else
    return GPGpath;
#endif
}

void GpgPlugin::clear()
{
    QList<DecryptMsg>::iterator it;
    for (it = m_decrypt.begin(); it != m_decrypt.end();){
        if (it->msg){
            ++it;
            continue;
        }
        delete it->process;
        QFile::remove(it->infile);
        QFile::remove(it->outfile);
        m_decrypt.erase(it);
        it = m_decrypt.begin();
    }
    for (it = m_import.begin(); it != m_import.end(); ){
        if (it->msg){
            ++it;
            continue;
        }
        delete it->process;
        QFile::remove(it->infile);
        QFile::remove(it->outfile);
        m_import.erase(it);
        it = m_import.begin();
    }
    for (it = m_public.begin(); it != m_public.end(); ){
        if (it->contact){
            ++it;
            continue;
        }
        delete it->process;
        QFile::remove(it->infile);
        QFile::remove(it->outfile);
        m_public.erase(it);
        it = m_public.begin();
    }
}

void GpgPlugin::decryptReady()
{
    int res = 0;
    for (QList<DecryptMsg>::iterator it = m_decrypt.begin(); it != m_decrypt.end(); ++it){
        QProcess *p = it->process;
        if (p && p->state() != QProcess::Running && it->msg){
            Message *msg = it->msg;
            it->msg = NULL;
            QTimer::singleShot(0, this, SLOT(clear()));
            if (p->exitStatus() == QProcess::NormalExit && p->exitCode() == 0){
                QString s = it->outfile;
                QFile f(s);
                if (f.open(QIODevice::ReadOnly)){
                    QByteArray ba = f.readAll();
                    msg->setText(QString::fromUtf8(ba.data(), ba.size()));
                    msg->setFlags(msg->getFlags() | MESSAGE_SECURE);
                }else{
                    log(L_WARN, "Can't open output decrypt file %s", qPrintable(s));
                    res = -1;
                }
                if (!it->key.isEmpty()){
                    unsigned i = 1;
                    for (i = 1; i <= value("NPassphrases").toUInt(); i++){
                        if (it->key == value("Keys").toStringList()[i])
                            break;
                    }
                    if (i > value("NPassphrases").toUInt()){
                        setValue("NPassphrases", i);
                        QStringList l = value("Keys").toStringList();
                        l[i] = it->key;
                        setValue("Keys", l);
                    }

                    QStringList list = value("Passphrases").toStringList();
                    list[i] = it->passphrase;
                    setValue("Passphrases", list);
                    for (;;){
                        QList<DecryptMsg>::iterator itw;
                        bool bDecode = false;
                        for (itw = m_wait.begin(); itw != m_wait.end(); ++itw){
                            if (itw->key == it->key){
                                decode(itw->msg, it->passphrase, it->key);
                                m_wait.erase(itw);
                                bDecode = true;
                                break;
                            }
                        }
                        if (!bDecode)
                            break;
                    }
                    if (m_passphraseDlg && (it->key == m_passphraseDlg->m_key)){
                        delete m_passphraseDlg;
                        m_passphraseDlg = NULL;
                        askPassphrase();
                    }
                }
            }else{
                QString key;
                QString passphrase;
				p->setReadChannel(QProcess::StandardError);
                QByteArray ba = p->readAll();
                QString str = QString::fromUtf8(ba.data(), ba.size());
                while(!str.isEmpty()) {
                    key = getToken(str, '\n');
                    if (key.contains("BAD_PASSPHRASE")){
                        int n = key.indexOf("BAD_PASSPHRASE ");
                        if(n < 0)
                            break;
                        key = key.mid(n + strlen("BAD_PASSPHRASE "));
                        if (m_passphraseDlg && (it->key == m_passphraseDlg->m_key)){
                            DecryptMsg m;
                            m.msg    = msg;
                            m.key    = key;
                            m_wait.push_back(m);
                            m_passphraseDlg->error();
                            return;
                        }
                        if (it->passphrase.isEmpty()){
                            for (unsigned i = 1; i <= value("NPassphrases").toUInt(); i++){
                                if (key == value("Keys").toStringList()[i]){
                                    passphrase = value("Passphrases").toStringList()[i];
                                    break;
                                }
                            }
                        }
                        if (it->passphrase.isEmpty() && !passphrase.isEmpty()){
                            if (decode(msg, passphrase, key))
                                return;
                        }else{
                            DecryptMsg m;
                            m.msg    = msg;
                            m.key    = key;
                            m_wait.push_back(m);
                            it->msg = NULL;
                            QTimer::singleShot(0, this, SLOT(clear()));
                            askPassphrase();
                            return;
                        }
                    }
                }
                if (m_passphraseDlg && (it->key == m_passphraseDlg->m_key)){
                    delete m_passphraseDlg;
                    m_passphraseDlg = NULL;
                    askPassphrase();
                } else {
                    msg->setText(key + '\n' + str);
                }
            }
            EventMessageReceived e(msg);
            if ((res == 0) && processEvent(&e))
                return;
            if (!e.process(this))
                delete msg;
            return;
        }
    }
    log(L_WARN, "No decrypt exec");
}

void GpgPlugin::importReady()
{
    for (QList<DecryptMsg>::iterator it = m_import.begin(); it != m_import.end(); ++it){
        QProcess *p = it->process;
        if (p && p->state() != QProcess::Running){
            Message *msg = new Message(MessageGPGKey);
            msg->setContact(it->msg->contact());
            msg->setClient(it->msg->client());
            msg->setFlags(it->msg->getFlags());

			p->setReadChannel(QProcess::StandardError);
            QByteArray ba = p->readAll();
            QString err = QString::fromLocal8Bit(ba.data(), ba.size());
            if (p->exitStatus() == QProcess::NormalExit && p->exitCode() == 0){
                QRegExp r1("[0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F]:");
                QRegExp r2("\".*\"");
                int len;
                int pos = r1.indexIn(err, 0);
				len = r1.matchedLength();
                if (pos >= 0)
				{
                    QString key_name;
                    key_name  = err.mid(pos + 1, len - 2);
                    QString text = key_name;
                    text += ' ';
                    pos = r2.indexIn(err, 0);
					len = r2.matchedLength();
                    text += err.mid(pos + 1, len - 2);
                    msg->setText(text);
                    delete it->msg;
                    it->msg = msg;

                    QString home = GpgPlugin::plugin->getHomeDir();

                    QStringList sl;
                    sl += GPG();
                    sl += "--no-tty";
                    sl += "--homedir";
                    sl += home;
                    sl += value("PublicList").toString().split(' ');

                    QProcess *proc = new QProcess(this);

                    DecryptMsg dm;
                    dm.process = proc;
                    dm.contact = msg->contact();
                    dm.outfile = key_name;
                    m_public.push_back(dm);
                    connect(dm.process, SIGNAL(processExited()), this, SLOT(publicReady()));
                    dm.process->start(sl.join(" "));
                } else {
                    QString str;
                    if(!err.isEmpty())
                        str = '(' + err + ')';
                    msg->setText(i18n("Importing public key failed") + str);
                }
            } else {
                QString str;
                if(!err.isEmpty())
                    str = '(' + err + ')';
                msg->setText(i18n("Importing public key failed") + str);
            }
            EventMessageReceived e(it->msg);
            if (!e.process(this))
                delete it->msg;
            it->msg = NULL;
            QTimer::singleShot(0, this, SLOT(clear()));
            return;
        }
    }
    log(L_WARN, "No decrypt exec");
}

QByteArray GpgPlugin::getConfig()
{
	/*
    QStringList keys;
    QStringList passphrases;
    for (unsigned i = 1; i <= getnPassphrases(); i++){
        keys.append(getKeys(i));
        passphrases.append(getPassphrases(i));
    }
    if (!getSavePassphrase()){
        clearKeys();
        clearPassphrases();
    }
    QByteArray res = save_data(gpgData, &data);
    for (unsigned i = 0; i < getnPassphrases(); i++){
        setKeys(i + 1, keys[i]);
        setPassphrases(i + 1, passphrases[i]);
    }
	*/
    return QByteArray();
}

bool GpgPlugin::processEvent(Event *e)
{
    switch (e->type()){
    case eEventCheckCommandState:{
            EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
            CommandDef *cmd = ecs->cmd();
            if (cmd->menu_id == MenuMessage){
                if (cmd->id == MessageGPGKey){
                    cmd->flags &= ~COMMAND_CHECKED;
                    CommandDef c = *cmd;
                    c.id = MessageGeneric;
                    return EventCheckCommandState(&c).process();
                }
                if (cmd->id == MessageGPGUse){
                    cmd->flags &= ~COMMAND_CHECKED;
                    Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
                    if (contact == NULL)
                        return false;
                    SIM::PropertyHubPtr data = contact->getUserData("gpg", false);
                    if (!data || data->value("Key").toString().isEmpty())
                        return false;
                    if (data->value("Use").toBool())
                        cmd->flags |= COMMAND_CHECKED;
                    return true;
                }
            }
            return false;
        }
    case eEventCommandExec:{
            EventCommandExec *ece = static_cast<EventCommandExec*>(e);
            CommandDef *cmd = ece->cmd();
            if ((cmd->menu_id == MenuMessage) && (cmd->id == MessageGPGUse)){
                Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
                if (contact == NULL)
                    return false;
                SIM::PropertyHubPtr data = contact->getUserData("gpg", false);
                if (data && !data->value("Key").toString().isEmpty())
                    data->setValue("Use", (cmd->flags & COMMAND_CHECKED) != 0);
                return true;
            }
            return false;
        }
    case eEventCheckSend:{
            EventCheckSend *ecs = static_cast<EventCheckSend*>(e);
            if ((ecs->id() == MessageGPGKey) && ecs->client()->canSend(MessageGeneric, ecs->data()))
                return true;
            return false;
        }
    case eEventMessageSent:{
            EventMessage *em = static_cast<EventMessage*>(e);
            Message *msg = em->msg();
            for (QList<KeyMsg>::iterator it = m_sendKeys.begin(); it != m_sendKeys.end(); ++it){
                if (it->msg == msg){
                    if (msg->getError().isEmpty()){
                        Message m(MessageGPGKey);
                        m.setText(it->key);
                        m.setClient(msg->client());
                        m.setContact(msg->contact());
                        EventSent(&m).process();
                    }
                    m_sendKeys.erase(it);
                    break;
                }
            }
            return false;
        }
    case eEventMessageSend:{
            EventMessage *em = static_cast<EventMessage*>(e);
            Message *msg = em->msg();
            if (msg->type() == MessageGeneric){
                Contact *contact = getContacts()->contact(msg->contact());
                if (contact){
                    SIM::PropertyHubPtr data = contact->getUserData("gpg", false);
                    if (data && !data->value("Key").toString().isEmpty() && data->value("Use").toBool()){
                        msg->setFlags(msg->getFlags() | MESSAGE_SECURE);
                        if (msg->getFlags() & MESSAGE_RICHTEXT){
                            QString text = msg->getPlainText();
                            msg->setText(text);
                            msg->setFlags(msg->getFlags() & ~MESSAGE_RICHTEXT);
                        }
                    }
                }
            }
            return false;
        }
    case eEventSend:{
            EventSend *es = static_cast<EventSend*>(e);
            if ((es->msg()->type() == MessageGeneric) &&
                (es->msg()->getFlags() & MESSAGE_SECURE)){
                Contact *contact = getContacts()->contact(es->msg()->contact());
                if (contact)
                {
                    SIM::PropertyHubPtr data = contact->getUserData("gpg", false);
                    if (data && !data->value("Key").toString().isEmpty() && data->value("Use").toBool()){
                        QString output = user_file("m.");
                        output += QString::number((unsigned long)es->msg());
                        QString input = output + ".in";
                        QFile in(input);
                        if (!in.open(QIODevice::WriteOnly | QIODevice::Truncate)){
                            log(L_WARN, "Can't create %s", qPrintable(input));
                            return false;
                        }
                        in.write(es->localeText());
                        in.close();
                        QString home = GpgPlugin::plugin->getHomeDir();

                        QStringList sl;
                        sl += GPG();
                        sl += "--no-tty";
                        sl += "--homedir";
                        sl += home;
                        sl += value("Encrypt").toString().split(' ');
                        sl = sl.replaceInStrings(QRegExp("\\%plainfile\\%"), input);
                        sl = sl.replaceInStrings(QRegExp("\\%cipherfile\\%"), output);
                        sl = sl.replaceInStrings(QRegExp("\\%userid\\%"), data->value("Key").toString());

                        QProcess proc(this);

                        proc.start(sl.join(" "));

                        // FIXME: not soo good...
                        while(proc.state() == QProcess::Running)
                            qApp->processEvents();

                        if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0){
                            es->msg()->setError(I18N_NOOP("Encrypt failed"));
                            QFile::remove(input);
                            QFile::remove(output);
                            return true;
                        }
                        QFile::remove(input);
                        QFile out(output);
                        if (!out.open(QIODevice::ReadOnly)){
                            QFile::remove(output);
                            es->msg()->setError(I18N_NOOP("Encrypt failed"));
                            return true;
                        }
                        es->setLocaleText(QByteArray(out.readAll()));
                        out.close();
                        QFile::remove(output);
                        return false;
                    }
                }
            }
            return false;
        }
    case eEventMessageReceived:{
            EventMessage *em = static_cast<EventMessage*>(e);
            Message *msg = em->msg();
            if(!msg)
                return false;
            if ((msg->baseType() == MessageGeneric) && m_bMessage){
                QString text = msg->getPlainText();
                const char SIGN_MSG[] = "-----BEGIN PGP MESSAGE-----";
                const char SIGN_KEY[] = "-----BEGIN PGP PUBLIC KEY BLOCK-----";
                if (text.startsWith(SIGN_MSG)){
                    if (decode(msg, "", ""))
                        return true;
                    return false;
                }
                if (text.startsWith(SIGN_KEY)){
                    QString input = user_file("m.");
                    input  += QString::number((unsigned long)msg);
                    input += ".in";
                    QFile in(input);
                    if (!in.open(QIODevice::WriteOnly | QIODevice::Truncate)){
                        log(L_WARN, "Can't create %s", qPrintable(input));
                        return false;
                    }
                    QByteArray cstr = text.toUtf8();
                    in.write(cstr);
                    in.close();
                    QString home = GpgPlugin::plugin->getHomeDir();

                    QStringList sl;
                    sl += GPG();
                    sl += "--no-tty";
                    sl += "--homedir";
                    sl += home;
                    sl += value("Import").toString().split(' ');
                    sl = sl.replaceInStrings(QRegExp("\\%keyfile\\%"), input);

                    QProcess *proc = new QProcess(this);

                    DecryptMsg dm;
                    dm.process = proc;
                    dm.msg     = msg;
                    dm.infile  = input;
                    m_import.push_back(dm);
                    connect(dm.process, SIGNAL(processExited()), this, SLOT(importReady()));
                    dm.process->start(sl.join(" "));
					dm.process->write("\n");
                    return msg;
                }
            }
            return false;
        }
	case eEventPluginLoadConfig:
	{
        setPropertyHub(ProfileManager::instance()->getPropertyHub("gpg"));
        // Defaults:
        if(!value("Home").isValid())
                setValue("Home", "keys/");
        if(!value("GenKey").isValid())
                setValue("GenKey", "--gen-key --batch");
        if(!value("PublicList").isValid())
                setValue("PublicList", "--with-colon --list-public-keys");
        if(!value("SecretList").isValid())
                setValue("SecretList", "--with-colon --list-secret-keys");
        if(!value("Import").isValid())
                setValue("Import", "--import %keyfile%");
        if(!value("Export").isValid())
                setValue("Export", "--batch --yes --armor --comment \"\" --no-version --export %userid%");
        if(!value("Encrypt").isValid())
                setValue("Encrypt", "--charset utf8 --batch --yes --armor --comment \"\" --no-version --recipient %userid% --trusted-key %userid% --output %cipherfile% --encrypt %plainfile%");
        if(!value("Decrypt").isValid())
                setValue("Decrypt", "--charset utf8 --yes --passphrase-fd 0 --status-fd 2 --output %plainfile% --decrypt %cipherfile%");
        break;
	}
    default:
        break;
    }
    return false;
}

static unsigned decode_index = 0;

bool GpgPlugin::decode(Message *msg, const QString &aPassphrase, const QString &key)
{
    QString output = user_file("md.");
    output += QString::number(decode_index++);
    QString input = output + ".in";
    QFile in(input);
    if (!in.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        log(L_WARN, "Can't create %s", qPrintable(input));
        return false;
    }
    QByteArray cstr = msg->getPlainText().toUtf8();
    in.write(cstr);
    in.close();
    QString home = GpgPlugin::plugin->getHomeDir();

    QStringList sl;
    sl += GPG();
    sl += "--no-tty";
    sl += "--homedir";
    sl += home;
    sl += value("Decrypt").toString().split(' ');
    sl = sl.replaceInStrings(QRegExp("\\%plainfile\\%"), output);
    sl = sl.replaceInStrings(QRegExp("\\%cipherfile\\%"), input);

    QProcess *proc = new QProcess(this);

    DecryptMsg dm;
    dm.process = proc;
    dm.msg     = msg;
    dm.infile  = input;
    dm.outfile = output;
    dm.passphrase = aPassphrase;
    dm.key        = key;
    m_decrypt.push_back(dm);

    connect(dm.process, SIGNAL(processExited()), this, SLOT(decryptReady()));
    dm.process->start(sl.join(" "));
	dm.process->write(aPassphrase.toUtf8().data());
    return true;
}

void GpgPlugin::publicReady()
{
    for (QList<DecryptMsg>::iterator it = m_public.begin(); it != m_public.end(); ++it){
        QProcess *p = it->process;
        if (p && p->state() != QProcess::Running){
            if (p->exitStatus() == QProcess::NormalExit && p->exitCode() == 0){
				p->setReadChannel(QProcess::StandardError);
                QByteArray str(p->readAll());
                for (;;){
                    QByteArray line;
                    line = getToken(str, '\n');
                    if(line.isEmpty())
                        break;
                    QByteArray type = getToken(line, ':');
                    if (type == "pub"){
                        getToken(line, ':');
                        getToken(line, ':');
                        getToken(line, ':');
                        QString sign = getToken(line, ':');
                        QString name = it->outfile;
                        int pos = sign.length() - name.length();
                        if (pos < 0)
                            pos = 0;
                        if (sign.mid(pos) == name.toLatin1()){
                            Contact *contact = getContacts()->contact(it->contact);
                            if (contact)
                            {
                                SIM::PropertyHubPtr data = contact->getUserData("gpg", true);
                                data->setValue("Key", sign);
                            }
                            break;
                        }
                    }
                }
            }
            it->contact = 0;
            break;
        }
    }
}

void GpgPlugin::passphraseApply(const QString &passphrase)
{
    for (QList<DecryptMsg>::iterator it = m_wait.begin(); it != m_wait.end(); ++it){
        if (it->key == m_passphraseDlg->m_key){
            Message *msg = it->msg;
            m_wait.erase(it);
            decode(msg, passphrase, m_passphraseDlg->m_key);
            return;
        }
    }
    delete m_passphraseDlg;
    m_passphraseDlg = NULL;
    askPassphrase();
}

QWidget *GpgPlugin::createConfigWindow(QWidget *parent)
{
    return new GpgCfg(parent, this);
}

void GpgPlugin::reset()
{
    if (!GPG().isEmpty() && !value("Home").toString().isEmpty() && !value("Key").toString().isEmpty()){
#ifdef HAVE_CHMOD
        chmod(QFile::encodeName(user_file(value("Home").toString())), 0700);
#endif
        registerMessage();
    }else{
        unregisterMessage();
    }
}

QString GpgPlugin::getHomeDir()
{
    QString home = user_file(value("Home").toString());
    if (home.endsWith("\\") || home.endsWith("/"))
        home = home.left(home.length() - 1);
    return home;
}

#if 0
i18n("%n GPG key", "%n GPG keys", 1);
#endif

static Message *createGPGKey(Buffer *cfg)
{
    return new Message(MessageGPGKey, cfg);
}

static QObject *generateGPGKey(MsgEdit *p, Message *msg)
{
    return new MsgGPGKey(p, msg);
}

static MessageDef defGPGKey =
    {
        NULL,
        NULL,
        MESSAGE_INFO | MESSAGE_SYSTEM,
        "%n GPG key",
        "%n GPG keys",
        createGPGKey,
        generateGPGKey,
        NULL
    };

static MessageDef defGPGUse =
    {
        NULL,
        NULL,
        MESSAGE_SILENT,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    };

static QWidget *getGpgSetup(QWidget *parent, void *data)
{
    return new GpgUser(parent, (GpgUserData*)data);
}

void GpgPlugin::registerMessage()
{
    if (m_bMessage)
        return;
    m_bMessage = true;
    Command cmd;
    cmd->id			 = MessageGPGKey;
    cmd->text		 = I18N_NOOP("GPG key");
    cmd->icon		 = "encrypted";
    cmd->param		 = &defGPGKey;
    cmd->menu_grp	= 0x4081;
    EventCreateMessageType(cmd).process();

    cmd->id			 = MessageGPGUse;
    cmd->text		 = I18N_NOOP("Use GPG encryption");
    cmd->icon		 = QString::null;
    cmd->param		 = &defGPGUse;
    cmd->menu_grp	 = 0x4080;
    EventCreateMessageType(cmd).process();

    cmd->id		 = user_data_id;
    cmd->text	 = I18N_NOOP("&GPG key");
    cmd->icon	 = "encrypted";
    cmd->param	 = (void*)getGpgSetup;
    EventAddPreferences(cmd).process();
}

void GpgPlugin::unregisterMessage()
{
    if (!m_bMessage)
        return;
    m_bMessage = false;
    EventRemoveMessageType(MessageGPGKey).process();
    EventRemoveMessageType(MessageGPGUse).process();
    EventRemovePreferences(user_data_id).process();
}

void GpgPlugin::askPassphrase()
{
    if (m_passphraseDlg || m_wait.empty())
        return;
    m_passphraseDlg = new PassphraseDlg(this, m_wait.front().key);
    connect(m_passphraseDlg, SIGNAL(finished()), this, SLOT(passphraseFinished()));
    connect(m_passphraseDlg, SIGNAL(apply(const QString&)), this, SLOT(passphraseApply(const QString&)));
    raiseWindow(m_passphraseDlg);
}

void GpgPlugin::passphraseFinished()
{
    if (m_passphraseDlg){
        for (QList<DecryptMsg>::iterator it = m_wait.begin(); it != m_wait.end();){
            if (it->key != m_passphraseDlg->m_key){
                ++it;
                continue;
            }
            EventMessageReceived e(it->msg);
            if (!e.process(this))
                delete it->msg;
            m_wait.erase(it);
            it = m_wait.begin();
        }
    }
    m_passphraseDlg = NULL;
    askPassphrase();
}

void GpgPlugin::setPropertyHub(SIM::PropertyHubPtr hub)
{
	m_propertyHub = hub;
}

SIM::PropertyHubPtr GpgPlugin::propertyHub()
{
	return m_propertyHub;
}

QVariant GpgPlugin::value(const QString& key)
{
	return m_propertyHub->value(key);
}

void GpgPlugin::setValue(const QString& key, const QVariant& v)
{
	m_propertyHub->setValue(key, v);
}

MsgGPGKey::MsgGPGKey(MsgEdit *parent, Message *msg)
        : QObject(parent)
{
    m_client = msg->client();
    m_edit   = parent;
    m_edit->m_edit->setText(QString::null);
    m_edit->m_edit->setReadOnly(true);
    m_edit->m_edit->setParam(m_edit);

    Command cmd;
    cmd->id    = CmdSend;
    cmd->flags = COMMAND_DISABLED;
    cmd->param = m_edit;
    EventCommandDisabled(cmd).process();

    QString gpg  = GpgPlugin::plugin->GPG();
    QString home = GpgPlugin::plugin->getHomeDir();
    m_key = GpgPlugin::plugin->value("Key").toString();

    QStringList sl;
    sl += GpgPlugin::plugin->GPG();
    sl += "--no-tty";
    sl += "--homedir";
    sl += home;
    sl += GpgPlugin::plugin->value("Export").toString().split(' ');
    sl = sl.replaceInStrings(QRegExp("\\%userid\\%"), m_key);

    m_process = new QProcess(this);

    connect(m_process, SIGNAL(processExited()), this, SLOT(exportReady()));
	m_process->start(sl.join(" "));
	exportReady();
}

MsgGPGKey::~MsgGPGKey()
{
    delete m_process;
}

void MsgGPGKey::init()
{
    m_edit->m_edit->setFocus();
}

void MsgGPGKey::exportReady()
{
    if (m_process->exitStatus() == QProcess::NormalExit && m_process->exitCode() == 0) {
		m_process->setReadChannel(QProcess::StandardOutput);
        QByteArray ba1 = m_process->readAll();
        m_edit->m_edit->setText(QString::fromLocal8Bit(ba1.data(), ba1.size()));
        if(ba1.isEmpty()) {
			m_process->setReadChannel(QProcess::StandardError);
            QByteArray ba2 = m_process->readAll();
            QString errStr;
            if(!ba2.isEmpty())
                errStr = " (" + QString::fromLocal8Bit( ba2.data(), ba2.size() ) + ") ";
            BalloonMsg::message(i18n("Can't read gpg key ") + errStr +
                                     " Error code: " + QString::number(m_process->error()), m_edit->m_edit);
        }
    }

    Command cmd;
    cmd->id    = CmdSend;
    cmd->flags = 0;
    cmd->param = m_edit;
    EventCommandDisabled(cmd).process();

    delete m_process;
    m_process = 0;
}

bool MsgGPGKey::processEvent(Event *e)
{
    if (e->type() == eEventCheckCommandState){
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if (cmd->param == m_edit){
            unsigned id = cmd->bar_grp;
            if ((id >= MIN_INPUT_BAR_ID) && (id < MAX_INPUT_BAR_ID)){
                cmd->flags |= BTN_HIDE;
                return true;
            }
            switch (cmd->id){
            case CmdSend:
            case CmdSendClose:
                e->process(this);
                cmd->flags &= ~BTN_HIDE;
                return true;
            case CmdTranslit:
            case CmdSmile:
            case CmdNextMessage:
            case CmdMsgAnswer:
                e->process(this);
                cmd->flags |= BTN_HIDE;
                return true;
            }
        }
    }
    if (e->type() == eEventCommandExec){
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if ((cmd->id == CmdSend) && (cmd->param == m_edit)){
            QString msgText = m_edit->m_edit->toPlainText();
            if (!msgText.isEmpty()){
                Message *msg = new Message;
                msg->setText(msgText);
                msg->setContact(m_edit->m_userWnd->id());
                msg->setClient(m_client);
                msg->setFlags(MESSAGE_NOHISTORY);
                KeyMsg km;
                km.key = m_key;
                km.msg = msg;
                GpgPlugin::plugin->m_sendKeys.push_back(km);
                EventRealSendMessage(msg, m_edit).process();
            }
            return true;
        }
    }
    return false;
}

