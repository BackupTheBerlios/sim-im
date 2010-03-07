/***************************************************************************
                          msgedit.cpp  -  description
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

#include "msgedit.h"

#include "simapi.h"
#include "contacts/clientdataiterator.h"
#include "contacts/contact.h"
#include "contacts/client.h"

#include "userwnd.h"
#include "simgui/toolbtn.h"

#include "log.h"
#include "msgrecv.h"
#include "msgsms.h"
#include "msgurl.h"
#include "msgcontacts.h"
#include "simgui/listview.h"
#include "msgfile.h"
#include "msgauth.h"
#include "userlist.h"
#include "simgui/ballonmsg.h"
#include "container.h"
#include "icons.h"
#include "history.h"

#include <algorithm>
#include <QVBoxLayout>
#include <QTimer>
#include <QApplication>
#include <QClipboard>
#include <algorithm>

using namespace std;
using namespace SIM;

const unsigned NO_TYPE = (unsigned)(-1);

MsgTextEdit::MsgTextEdit(MsgEdit *edit, QWidget *parent)
        : TextEdit(parent)
{
    m_edit = edit;
    setBackground(CorePlugin::instance()->value("EditBackground").toUInt());
    setForeground(CorePlugin::instance()->value("EditForeground").toUInt(), true);
#if defined(USE_KDE)
#if KDE_IS_VERSION(3,2,0)
    setCheckSpellingEnabled(CorePlugin::instance()->getEnableSpell());
#endif
#endif
}

void MsgTextEdit::contextMenuEvent(QContextMenuEvent* event)
{
    if (m_bInClick)
        return;

    m_popupPos = event->pos();

    Command cmd;

    cmd->popup_id	= MenuTextEdit;
    cmd->param		= (TextEdit*)this;
    cmd->flags		= COMMAND_NEW_POPUP;
    EventMenuGet e(cmd);
    e.process();
    if(e.menu())
        e.menu()->exec(event->globalPos());
}

Message *MsgTextEdit::createMessage(QMimeSource *src)
{
    Message *msg = NULL;
    CommandDef *cmd;
    CommandsMapIterator it(CorePlugin::instance()->messageTypes);
    while ((cmd = ++it) != NULL){
        MessageDef *def = (MessageDef*)(cmd->param);
        if (def && def->drag){
            msg = def->drag(src);
            if (msg){
                Command c;
                c->id      = cmd->id;
                c->menu_id = MenuMessage;
                c->param	 = (void*)(m_edit->m_userWnd->id());
                if (EventCheckCommandState(c).process())
                    break;
                delete msg;
                msg = NULL;
            }
        }
    }
    return msg;
}

void MsgTextEdit::contentsDropEvent(QDropEvent *e)
{
    Message *msg = createMessage(e);
    if (msg){
        e->accept();
        msg->setContact(m_edit->m_userWnd->id());
        EventOpenMessage(msg).process();
        delete msg;
        return;
    }
    //TextEdit::contentsDropEvent(e); //FIXME
}

void MsgTextEdit::contentsDragEnterEvent(QDragEnterEvent *e)
{
    Message *msg = createMessage(e);
    if (msg){
        delete msg;
        e->accept();
        return;
    }
    //TextEdit::contentsDragEnterEvent(e);  //FIXME
}

void MsgTextEdit::contentsDragMoveEvent(QDragMoveEvent *e)
{
    Message *msg = createMessage(e);
    if (msg){
        delete msg;
        e->accept();
        return;
    }
    //TextEdit::contentsDragMoveEvent(e); //FIXME
}

MsgEdit::MsgEdit(QWidget *parent, UserWnd *userWnd) : QFrame(parent)
{
    m_userWnd	= userWnd;
    m_msg		= NULL;
    m_bTyping	= false;
    m_type		= NO_TYPE;
    m_flags		= 0;
    m_retry.msg = NULL;
    m_bReceived = false;
    m_processor = NULL;
    m_recvProcessor = NULL;
    m_cmd.param = NULL;

    connect(CorePlugin::instance(), SIGNAL(modeChanged()), this, SLOT(modeChanged()));

    m_layout = new QVBoxLayout(this);
    m_layout->setMargin(0);

    m_edit = new MsgTextEdit(this, this);
    m_edit->setBackground(QColor(CorePlugin::instance()->value("EditBackground").toUInt() & 0xFFFFFF));
    m_edit->setBackground(QColor(255, 255, 255));
    m_edit->setForeground(QColor(CorePlugin::instance()->value("EditForeground").toUInt() & 0xFFFFFF), true);
    m_edit->setFont(CorePlugin::instance()->editFont);
    m_edit->setCtrlMode(!CorePlugin::instance()->value("SendOnEnter").toBool());
    m_edit->setParam(this);
    setFocusProxy(m_edit);

    connect(m_edit, SIGNAL(lostFocus()), this, SLOT(editLostFocus()));
    connect(m_edit, SIGNAL(textChanged()), this, SLOT(editTextChanged()));
    connect(m_edit, SIGNAL(ctrlEnterPressed()), this, SLOT(editEnterPressed()));
    connect(m_edit, SIGNAL(colorsChanged()), this, SLOT(colorsChanged()));
    connect(m_edit, SIGNAL(finished()), this, SLOT(editFinished()));
    connect(m_edit, SIGNAL(fontSelected(const QFont&)), this, SLOT(editFontChanged(const QFont&)));

    QFontMetrics fm(m_edit->font());
    m_edit->setMinimumSize(QSize(fm.maxWidth(), fm.height() + 10));

    EventToolbar e(ToolBarMsgEdit, NULL);
    e.process();
    m_bar = e.toolBar();
    m_bar->setParam(this);

    m_layout->addWidget(m_bar);
    m_layout->addWidget(m_edit);

    if (CorePlugin::instance()->getContainerMode() == 0)
        showCloseSend(false);
}

MsgEdit::~MsgEdit()
{
    typingStop();
    editLostFocus();
    if (m_retry.msg)
        delete m_retry.msg;
    emit finished();
}

void MsgEdit::editFinished()
{
    m_edit = NULL;
}

void MsgEdit::execCommand(CommandDef *cmd)
{
    if (m_cmd.param){
        Message *msg = (Message*)(m_cmd.param);
        delete msg;
    }
    m_cmd = *cmd;
    QTimer::singleShot(0, this, SLOT(execCommand()));
}

void MsgEdit::execCommand()
{
    if (m_cmd.param == NULL)
        return;
    Message *msg = (Message*)(m_cmd.param);
    EventCommandExec(&m_cmd).process();
    delete msg;
    m_cmd.param = NULL;
    switch (m_cmd.id){
    case CmdMsgQuote:
    case CmdMsgForward:
        break;
    default:
        goNext();
    }
}

void MsgEdit::showCloseSend(bool bState)
{
    Command cmd;
    cmd->id			= CmdSendClose;
    cmd->text		= I18N_NOOP("C&lose after send");
    cmd->icon		= "exit";
    cmd->icon_on	= "exit";
    cmd->bar_grp	= 0x7010;
    cmd->flags		= bState ? COMMAND_DEFAULT : BTN_HIDE;
    cmd->param		= this;
    if (CorePlugin::instance()->value("CloseSend").toBool())
        cmd->flags |= COMMAND_CHECKED;
    EventCommandChange(cmd).process();
}

void MsgEdit::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    emit heightChanged(height());
}

void MsgEdit::editFontChanged(const QFont &f)
{
    if (!CorePlugin::instance()->value("EditSaveFont").toBool())
        return;
    CorePlugin::instance()->editFont = f;
}

bool MsgEdit::setMessage(Message *msg, bool bSetFocus)
{
    m_type = msg->type();
    m_userWnd->setMessageType(msg->type());
    m_resource  = msg->getResource();
    m_bReceived = msg->getFlags() & MESSAGE_RECEIVED;
    QObject *processor = NULL;
    MsgReceived *rcv = NULL;
    if (m_bReceived){
        if ((msg->getFlags() & MESSAGE_OPEN) || (CorePlugin::instance()->getContainerMode() == 0)){
            rcv = new MsgReceived(this, msg, true);
            processor = rcv;
        }else{
            if (m_recvProcessor == NULL){
                rcv = new MsgReceived(this, msg, false);
                m_recvProcessor = rcv;
            }
        }
    }else{
        QObject *(*create)(MsgEdit *custom, Message *msg) = NULL;
        CommandDef *cmd = CorePlugin::instance()->messageTypes.find(msg->baseType());
        if (cmd == NULL)
            return false;
        MessageDef *def = (MessageDef*)(cmd->param);
        if (def == NULL)
            return false;
        create = def->generate;
        if (create){
            m_userWnd->setStatus(QString::null);
            processor = create(this, msg);
        }
    }
    if (processor){
        if (m_recvProcessor){
            delete m_recvProcessor;
            m_recvProcessor = NULL;
        }
        if (m_processor){
            delete m_processor;
            m_processor = NULL;
        }
        m_processor = processor;
    }
    m_client = msg->client();

    Contact *contact = getContacts()->contact(m_userWnd->id());
    if (contact){
        EventContactClient(contact).process();
    }

    m_bar->checkState();
    if (rcv)
        rcv->init();
    Command cmd;
    cmd->id			= CmdMultiply;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= this;
    if (msg->getFlags() & MESSAGE_FORWARD){
        cmd->flags = COMMAND_CHECKED;
        m_userWnd->showListView(true);
    }
    EventCommandChecked(cmd).process();

    if (m_processor && bSetFocus)
        QTimer::singleShot(0, m_processor, SLOT(init()));
    return true;
}

Client *MsgEdit::client(void *&data, bool bCreate, bool bTyping, unsigned contact_id, bool bUseClient)
{
    data = NULL;
    Contact *contact = getContacts()->contact(contact_id);
    if (!bUseClient || m_client.isEmpty()){
        if (contact == NULL)
            return NULL;
        vector<ClientStatus> cs;
        getWays(cs, contact);
        unsigned i;
        for (i = 0; i < cs.size(); i++){
            Client *client = getContacts()->getClient(cs[i].client);
            if (client->canSend(m_type, cs[i].data)){
                data = cs[i].data;
                if (bTyping)
                    changeTyping(client, data);
                return client;
            }
        }
        for (i = 0; i < cs.size(); i++){
            for (unsigned n = 0; n < getContacts()->nClients(); n++){
                Client *client = getContacts()->getClient(n);
                clientData *d = cs[i].data;
                Contact *c;
                if (!client->isMyData(d, c))
                    continue;
                if (c && (c != contact))
                    continue;
                if (client->canSend(m_type, d)){
                    if (bCreate)
                        client->createData(cs[i].data, contact);
                    data = cs[i].data;
                    if (bTyping)
                        changeTyping(client, data);
                    return client;
                }
            }
        }
        if (bTyping)
            changeTyping(NULL, NULL);
        return NULL;
    }
    if (contact == NULL)
        return NULL;
    void *d;
    ClientDataIterator it(contact->clientData);
    while ((d = ++it) != NULL){
        if (it.client()->dataName(d) == m_client){
            data = d;
            if (bTyping)
                changeTyping(it.client(), data);
            return it.client();
        }
    }
    if (bTyping)
        changeTyping(NULL, NULL);
    return NULL;
}

void MsgEdit::setInput()
{
    if (m_recvProcessor){
        delete m_recvProcessor;
        m_recvProcessor = NULL;
        m_bar->checkState();
    }
}

static Message *createGeneric(Buffer *cfg)
{
    return new Message(MessageGeneric, cfg);
}

static QObject *generateGeneric(MsgEdit *w, Message *msg)
{
    return new MsgGen(w, msg);
}

#if 0
i18n("Message", "%n messages", 1);
#endif

static MessageDef defGeneric =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        "Message",
        "%n messages",
        createGeneric,
        generateGeneric,
        NULL
    };

static Message *createSMS(Buffer *cfg)
{
    return new SMSMessage(cfg);
}

static QObject* generateSMS(MsgEdit *w, Message *msg)
{
    return new MsgSMS(w, msg);
}

#if 0
i18n("SMS", "%n SMSs", 1);
#endif

static MessageDef defSMS =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        "SMS",
        "%n SMSs",
        createSMS,
        generateSMS,
        NULL
    };

#if 0
i18n("URL", "%n URLs", 1);
#endif

static Message *createUrl(Buffer *cfg)
{
    return new UrlMessage(MessageUrl, cfg);
}

static QObject *generateUrl(MsgEdit *p, Message *msg)
{
    return new MsgUrl(p, msg);
}

static Message *dropUrl(QMimeSource *src)
{
	log(L_DEBUG, "FIXME: Message:dropUrl");
	/*
    if (Q3UriDrag::canDecode(src)){
        QStringList l;
        if (Q3UriDrag::decodeLocalFiles(src, l))
            return NULL;
        if (!Q3UriDrag::decodeToUnicodeUris(src, l) || (l.count() < 1))
            return NULL;
        UrlMessage *msg = new UrlMessage;
        msg->setUrl(l[0]);
        return msg;
    }
	*/
    return NULL;
}

static MessageDef defUrl =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        "URL",
        "%n URLs",
        createUrl,
        generateUrl,
        dropUrl
    };

static Message *createContacts(Buffer *cfg)
{
    return new ContactsMessage(MessageContacts, cfg);
}

static QObject *generateContacts(MsgEdit *p, Message *msg)
{
    return new MsgContacts(p, msg);
}

static Message *dropContacts(QMimeSource *src)
{
    if (ContactDragObject::canDecode(src)){
        Contact *contact = ContactDragObject::decode(src);
        ContactsMessage *msg = new ContactsMessage;
        QString name = contact->getName();
        msg->setContacts(QString("sim:") + QString::number(contact->id()) + ',' + getToken(name, '/'));
        return msg;
    }
    return NULL;
}

#if 0
i18n("Contact list", "%n contact lists", 1);
#endif

static MessageDef defContacts =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        "Contact list",
        "%n contact lists",
        createContacts,
        generateContacts,
        dropContacts
    };

static Message *createFile(Buffer *cfg)
{
    return new FileMessage(MessageFile, cfg);
}

static QObject* generateFile(MsgEdit *w, Message *msg)
{
    return new MsgFile(w, msg);
}

Message *dropFile(QMimeSource *src)
{
	log(L_DEBUG, "FIXME: Message:dropFile");
	/*
    if (Q3UriDrag::canDecode(src)){
        QStringList files;
        if (Q3UriDrag::decodeLocalFiles(src, files) && files.count()){
            QString fileName;
            for (QStringList::Iterator it = files.begin(); it != files.end(); ++it){
                if (!fileName.isEmpty())
                    fileName += ',';
                fileName += '\"';
                fileName += *it;
                fileName += '\"';
            }
            FileMessage *m = new FileMessage;
            m->setFile(fileName);
            return m;
        }
    }
	*/
    return NULL;
}

#if 0
i18n("File", "%n files", 1);
#endif

static CommandDef fileCommands[] =
    {
        CommandDef (
            CmdFileAccept,
            I18N_NOOP("&Accept"),
            QString::null,
            QString::null,
            QString::null,
            ToolBarMsgEdit,
            0x1090,
            MenuMessage,
            0,
            0,
            COMMAND_CHECK_STATE,
            NULL,
            QString::null
        ),
        CommandDef (
            CmdFileDecline,
            I18N_NOOP("&Decline"),
            QString::null,
            QString::null,
            QString::null,
            ToolBarMsgEdit,
            0x1091,
            MenuMessage,
            0,
            MenuFileDecline,
            COMMAND_CHECK_STATE,
            NULL,
            QString::null
        ),
        CommandDef ()
    };

static MessageDef defFile =
    {
        fileCommands,
        NULL,
        MESSAGE_DEFAULT,
        "File",
        "%n files",
        createFile,
        generateFile,
        dropFile
    };

static Message *createAuthRequest(Buffer *cfg)
{
    return new AuthMessage(MessageAuthRequest, cfg);
}

static QObject* generateAuth(MsgEdit *w, Message *msg)
{
    return new MsgAuth(w, msg);
}

#if 0
i18n("Authorize request", "%n authorize requests", 1);
#endif

static CommandDef authRequestCommands[] =
    {
        CommandDef (
            CmdGrantAuth,
            I18N_NOOP("&Grant"),
            QString::null,
            QString::null,
            QString::null,
            ToolBarMsgEdit,
            0x1080,
            MenuMessage,
            0,
            0,
            COMMAND_DEFAULT,
            NULL,
            ""
        ),
        CommandDef (
            CmdRefuseAuth,
            I18N_NOOP("&Refuse"),
            QString::null,
            QString::null,
            QString::null,
            ToolBarMsgEdit,
            0x1081,
            MenuMessage,
            0,
            0,
            COMMAND_DEFAULT,
            NULL,
            QString::null
        ),
        CommandDef ()
    };

static MessageDef defAuthRequest =
    {
        authRequestCommands,
        NULL,
        MESSAGE_DEFAULT | MESSAGE_SYSTEM,
        "Authorize request",
        "%n authorize requests",
        createAuthRequest,
        generateAuth,
        NULL
    };

static Message *createAuthGranted(Buffer *cfg)
{
    return new AuthMessage(MessageAuthGranted, cfg);
}

#if 0
i18n("Authorization granted", "%n authorization granted", 1);
#endif

static MessageDef defAuthGranted =
    {
        NULL,
        NULL,
        MESSAGE_SILENT,
        "Authorization granted",
        "%n authorization granted",
        createAuthGranted,
        generateAuth,
        NULL
    };

static Message *createAuthRefused(Buffer *cfg)
{
    return new AuthMessage(MessageAuthRefused, cfg);
}

#if 0
i18n("Authorization refused", "%n authorization refused", 1);
#endif

static MessageDef defAuthRefused =
    {
        NULL,
        NULL,
        MESSAGE_SILENT | MESSAGE_ERROR,
        "Authorization refused",
        "%n authorization refused",
        createAuthRefused,
        generateAuth,
        NULL
    };

static Message *createAdded(Buffer *cfg)
{
    return new AuthMessage(MessageAdded, cfg);
}

#if 0
i18n("Add to contacts", "%n add to contacts", 1);
#endif

static MessageDef defAdded =
    {
        NULL,
        NULL,
        MESSAGE_INFO | MESSAGE_SYSTEM,
        "Add to contacts",
        "%n add to contacts",
        createAdded,
        NULL,
        NULL
    };

static Message *createRemoved(Buffer *cfg)
{
    return new AuthMessage(MessageRemoved, cfg);
}

#if 0
i18n("Removed from contacts", "%n removed from contacts", 1);
#endif

static MessageDef defRemoved =
    {
        NULL,
        NULL,
        MESSAGE_INFO | MESSAGE_SYSTEM,
        "Removed from contacts",
        "%n removed from contacts",
        createRemoved,
        NULL,
        NULL
    };

static Message *createStatus(Buffer *cfg)
{
    return new StatusMessage(cfg);
}

#if 0
i18n("Status changed", "%n times status changed", 1);
#endif

static MessageDef defStatus =
    {
        NULL,
        NULL,
        MESSAGE_HIDDEN,
        "Status changed",
        "%n times status changed",
        createStatus,
        NULL,
        NULL
    };

static bool cmp_status(ClientStatus s1, ClientStatus s2)
{
    if (s1.status > s2.status)
        return true;
    if (s1.status < s2.status)
        return false;
    if (s1.data->LastSend.toULong() > s2.data->LastSend.toULong())
        return true;
    if (s1.data->LastSend.toULong() < s2.data->LastSend.toULong())
        return false;
    return s1.client < s2.client;
}

void MsgEdit::getWays(vector<ClientStatus> &cs, Contact *contact)
{
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        ClientDataIterator it(contact->clientData, client);
        clientData *data;
        while ((data = ++it) != NULL){
            unsigned long status = STATUS_UNKNOWN;
            unsigned style = 0;
            QString statusIcon;
            client->contactInfo(data, status, style, statusIcon);
            ClientStatus s;
            s.client = i;
            s.status = status;
            s.data   = data;
            cs.push_back(s);
        }
    }
    sort(cs.begin(), cs.end(), cmp_status);
}

bool MsgEdit::sendMessage(Message *msg)
{
    if (m_retry.msg){
        delete m_retry.msg;
        m_retry.msg = NULL;
    }
    if (m_msg){
        delete msg;
        if (EventMessageCancel(m_msg).process())
            m_msg = NULL;
        stopSend(false);
        return false;
    }
    bool bClose = true;
    if (CorePlugin::instance()->getContainerMode()){
        bClose = false;
        Command cmd;
        cmd->id		= CmdSendClose;
        cmd->param	= this;
        EventCommandWidget eWidget(cmd);
        eWidget.process();
        QToolButton *btnClose = qobject_cast<QToolButton*>(eWidget.widget());
        if (btnClose)
            bClose = btnClose->isChecked();
    }
    CorePlugin::instance()->setValue("CloseSend", bClose);

    Contact *contact = getContacts()->contact(m_userWnd->id());
    if (contact){
		SIM::PropertyHubPtr data = contact->getUserData("translit");
        if (!data.isNull() && data->value("Translit").toBool())
            msg->setFlags(msg->getFlags() | MESSAGE_TRANSLIT);
    }

    msg->setFlags(msg->getFlags() | m_flags);
    m_flags = 0;

    if (m_userWnd->m_list){
        if( !m_userWnd->m_list->isHaveSelected() )
            return false;
        multiply = m_userWnd->m_list->selected();
        msg->setContact( multiply.first() );
        multiply.pop_front();
        msg->setClient(NULL);
        if( multiply.count() > 0 )
            msg->setFlags(msg->getFlags() | MESSAGE_MULTIPLY);
    }else if (!m_resource.isEmpty()){
        void *data = NULL;
        Client *c = client(data, true, false, msg->contact(), true);
        if (c){
            QString resources = c->resources(data);
            while (!resources.isEmpty()){
                QString res = getToken(resources, ';');
                getToken(res, ',');
                if (m_resource == res){
                    msg->setResource(m_resource);
                    break;
                }
            }
        }
    }

    editLostFocus();
    Command cmd;
    cmd->id		= CmdSend;
    cmd->text	= I18N_NOOP("Cancel");
    cmd->icon	= "cancel";
    cmd->flags	= BTN_PICT;
    cmd->param	= this;
    EventCommandChange(cmd).process();
    m_msg = msg;
    return send();
}

bool MsgEdit::send()
{
    Contact *contact = getContacts()->contact(m_msg->contact());
    QString client_str = m_msg->client();
    bool bSent = false;
    void *data = NULL;
    if (contact){
        EventMessageSend(m_msg).process();
        if (client_str.isEmpty()){
            m_type = m_msg->type();
            Client *c = client(data, true, false, m_msg->contact(), (m_msg->getFlags() & MESSAGE_MULTIPLY) == 0);
            if (c){
                m_msg->setClient(c->dataName(data));
                bSent = c->send(m_msg, data);
            }else{
                data = NULL;
                for (unsigned i = 0; i < getContacts()->nClients(); i++){
                    Client *client = getContacts()->getClient(i);
                    if (client->send(m_msg, NULL)){
                        bSent = true;
                        break;
                    }
                }
            }
        }else{
            ClientDataIterator it(contact->clientData);
            while ((data = ++it) != NULL){
                if (it.client()->dataName(data) == client_str){
                    if (it.client()->send(m_msg, data))
                        bSent = true;
                    break;
                }
            }
        }
    }
    if (bSent){
        if (data){
            ((clientData*)data)->LastSend.asULong() = QDateTime::currentDateTime().toTime_t();
        }
    }else{
        if (m_msg){
            delete m_msg;
            m_msg = NULL;
        }
        stopSend();
        Command cmd;
        cmd->id		= CmdSend;
        cmd->param	= this;
        EventCommandWidget eWidget(cmd);
        eWidget.process();
        QWidget *msgWidget = eWidget.widget();
        if (msgWidget == NULL)
            msgWidget = this;
        BalloonMsg::message(i18n("No such client for send message"), msgWidget);
        return false;
    }
    return true;
}

void MsgEdit::stopSend(bool bCheck)
{
    if (m_userWnd->m_list){
        Command cmd;
        m_userWnd->showListView(false);
        cmd->id			= CmdMultiply;
        cmd->text		= I18N_NOOP("Multi&ply send");
        cmd->icon		= "1rightarrow";
        cmd->icon_on	= "1leftarrow";
        cmd->flags		= COMMAND_DEFAULT;
        cmd->param		= this;
        EventCommandChange(cmd).process();
    }
    multiply.clear();
    Command cmd;
    cmd->id		    = CmdSend;
    cmd->text	    = I18N_NOOP("&Send");
    cmd->icon	    = "mail_generic";
    cmd->bar_id		= ToolBarMsgEdit;
    cmd->bar_grp	= 0x8000;
    cmd->flags		= BTN_PICT;
    cmd->param		= this;
    EventCommandChange(cmd).process();
    if (bCheck && (m_msg == NULL))
        return;
    if (m_msg)
        m_msg = NULL;
}

void MsgEdit::modeChanged()
{
    showCloseSend(CorePlugin::instance()->getContainerMode() != 0);
    m_edit->setCtrlMode(CorePlugin::instance()->value("SendOnEnter").toBool());
}

bool MsgEdit::setType(unsigned type)
{
    CommandDef *def;
    def = CorePlugin::instance()->messageTypes.find(type);
    if (def == NULL)
        return false;
    MessageDef *mdef = (MessageDef*)(def->param);
    if (mdef->flags & MESSAGE_SILENT)
        return false;
    if (mdef->create == NULL)
        return false;
    Message *msg = mdef->create(NULL);
    if (msg == NULL)
        return false;
    m_userWnd->setMessage(msg);
    delete msg;
    return true;
}

bool MsgEdit::adjustType()
{
    if (m_bReceived)
        return true;
    Command cmd;
    cmd->menu_id = MenuMessage;
    cmd->param = (void*)(m_userWnd->m_id);
    cmd->id = m_userWnd->getMessageType();
    if (m_userWnd->getMessageType() != m_type) {
        if(EventCheckCommandState(cmd).process()) {
            if (setType(m_userWnd->getMessageType()))
                return true;
        }
    }
    cmd->id = m_type;
    if(EventCheckCommandState(cmd).process())
        return true;
    EventMenuGetDef eMenu(MenuMessage);
    eMenu.process();
    CommandsDef *cmdsMsg = eMenu.defs();
    CommandsList itc(*cmdsMsg, true);
    CommandDef *c;
    unsigned desired = m_userWnd->getMessageType();
    bool bSet = false;
    while ((c = ++itc) != NULL){
        if (c->id == CmdContactClients)
            continue;
        c->param = (void*)(m_userWnd->m_id);
        if (!EventCheckCommandState(c).process())
            continue;
        if (setType(c->id)){
            bSet = true;
            break;
        }
    }
    m_userWnd->setMessageType(desired);
    return bSet;
}

bool MsgEdit::processEvent(Event *e)
{
    switch (e->type()) {
    case eEventContact: {
        EventContact *ec = static_cast<EventContact*>(e);
        if (ec->contact()->id() != m_userWnd->m_id)
            break;
        adjustType();
        break;
    }
    case eEventClientChanged: {
        adjustType();
        break;
    }
    case eEventMessageReceived: {
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        if (msg->getFlags() & MESSAGE_NOVIEW)
            return false;
        if ((msg->contact() == m_userWnd->id()) && (msg->type() != MessageStatus)){
            if (CorePlugin::instance()->getContainerMode()){
                bool bSetFocus = false;
                if (topLevelWidget() && topLevelWidget()->inherits("Container")){
                    Container *container = static_cast<Container*>(topLevelWidget());
                    if (container->wnd() == m_userWnd)
                        bSetFocus = true;
                }
                setMessage(msg, bSetFocus);
            }else{
                if (m_edit->isReadOnly())
                    QTimer::singleShot(0, this, SLOT(setupNext()));
            }
        }
        break;
    }
    case eEventRealSendMessage: {
        EventRealSendMessage *ersm = static_cast<EventRealSendMessage*>(e);
        if (ersm->edit() == this){
            sendMessage(ersm->msg());
            return true;
        }
        break;
    }
    case eEventCheckCommandState: {
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if ((cmd->param == (TextEdit*)m_edit) && (cmd->id == CmdTranslit)){
            Contact *contact = getContacts()->contact(m_userWnd->id());
            if (contact){
				SIM::PropertyHubPtr data = contact->getUserData("translit");
                if(!data.isNull()) {
                    cmd->flags &= ~COMMAND_CHECKED;
                    if (data->value("Translit").toBool())
                        cmd->flags |= COMMAND_CHECKED;
                    // FIXME: return true; missing here?
                }
            }
            return false;
        }
        if ((cmd->menu_id != MenuTextEdit) || (cmd->param != (TextEdit*)m_edit))
            return false;
        cmd->flags &= ~(COMMAND_CHECKED | COMMAND_DISABLED);
        switch (cmd->id){
        case CmdUndo:
            if (m_edit->isReadOnly())
                return false;
            if (!m_edit->document()->isUndoAvailable())
                cmd->flags |= COMMAND_DISABLED;
            return true;
        case CmdRedo:
            if (m_edit->isReadOnly())
                return false;
            if (!m_edit->document()->isRedoAvailable())
                cmd->flags |= COMMAND_DISABLED;
            return true;
        case CmdCut:
            if (m_edit->isReadOnly())
                return false;
        case CmdCopy:
            if (m_edit->textCursor().selectedText().isEmpty())
                cmd->flags |= COMMAND_DISABLED;
            return true;
        case CmdPaste:
            if (m_edit->isReadOnly())
                return false;
            if (QApplication::clipboard()->text().isEmpty())
                cmd->flags |= COMMAND_DISABLED;
            return true;
        case CmdClear:
            if (m_edit->isReadOnly())
                return false;
        case CmdSelectAll:
            if (m_edit->toPlainText().isEmpty())
                cmd->flags |= COMMAND_DISABLED;
            return true;
        }
        break;
    }
    case eEventCommandExec: {
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
#if defined(USE_KDE)
#if KDE_IS_VERSION(3,2,0)
        if (cmd->id == CmdEnableSpell){
            m_edit->setCheckSpellingEnabled(cmd->flags & COMMAND_CHECKED);
            return false;
        }
        else if ((cmd->id == CmdSpell) && (cmd->param == this)){
            m_edit->checkSpelling();
            return true;
        }
        else
#endif
#endif
        if ((cmd->id == CmdSmile) && (cmd->param == this)){
            EventCommandWidget eWidget(cmd);
            eWidget.process();
            QToolButton *btnSmile = qobject_cast<QToolButton*>(eWidget.widget());
            if (btnSmile){
                SmilePopup *popup = new SmilePopup(this);
                connect(popup, SIGNAL(insert(const QString &)), this, SLOT(insertSmile(const QString &)));
                QPoint p = CToolButton::popupPos(btnSmile, popup);
                popup->move(p);
                popup->show();
            }
            return true;
        }
        else if ((cmd->id == CmdTranslit) && (cmd->param == this)){
            Contact *contact = getContacts()->contact(m_userWnd->id());
            if (contact){
				SIM::PropertyHubPtr data = contact->getUserData("translit", true);
                data->setValue("Translit", ((cmd->flags & COMMAND_CHECKED) != 0));
            }
            return true;
        }
        else if ((cmd->id == CmdMultiply) && (cmd->param == this)){
            m_userWnd->showListView((cmd->flags & COMMAND_CHECKED) != 0);
            return true;
        }
        else if ((cmd->bar_id == ToolBarMsgEdit) && m_edit->isReadOnly() && (cmd->param == this)){
            switch (cmd->id){
            case CmdMsgAnswer:{
                    Message *msg = new Message(MessageGeneric);
                    msg->setContact(m_userWnd->id());
                    msg->setClient(m_client);
                    EventOpenMessage(msg).process();
                    delete msg;
                }
            case CmdNextMessage:
                QTimer::singleShot(0, this, SLOT(goNext()));
                break;
            }
        }
        else if ((cmd->menu_id != MenuTextEdit) || (cmd->param != this))
            return false;
        switch (cmd->id){
        case CmdUndo:
            m_edit->undo();
            return true;
        case CmdRedo:
            m_edit->redo();
            return true;
        case CmdCut:
            m_edit->cut();
            return true;
        case CmdCopy:
            m_edit->copy();
            return true;
        case CmdPaste:
            m_edit->paste();
            return true;
        case CmdClear:
            m_edit->clear();
            return true;
        case CmdSelectAll:
            m_edit->selectAll();
            return true;
        }
        break;
    }
    case eEventMessageSent:
    case eEventMessageAcked: {
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        if (msg == m_msg){
            QString err = msg->getError();
            if (!err.isEmpty())
                err = i18n(err);
            Contact *contact = getContacts()->contact(msg->contact());
            if(!err.isEmpty())
			{
                stopSend();
                Command cmd;
                cmd->id		= CmdSend;
                cmd->param	= this;
                EventCommandWidget eWidget(cmd);
                eWidget.process();
                QWidget *msgWidget = eWidget.widget();
                if (msgWidget == NULL)
                    msgWidget = this;
                if (msg->getRetryCode()){
                    m_retry.edit = this;
                    if (m_retry.msg)
                        delete m_retry.msg;
                    m_retry.msg  = new Message(msg->type());
                    m_retry.msg->setRetryCode(msg->getRetryCode());
                    m_retry.msg->setError(msg->getError());
                    EventMessageRetry e(&m_retry);
                    if (e.process())
                        return false;
                }else{
                    BalloonMsg::message(err, msgWidget);
                }
            }else{
                if (contact){
                    contact->setLastActive(QDateTime::currentDateTime().toTime_t());
                    EventContact(contact, EventContact::eStatus).process();
                }
                if (!multiply.empty() ){
                    CommandDef *def = CorePlugin::instance()->messageTypes.find(m_msg->type());
                    if (def){
                        MessageDef *mdef = (MessageDef*)(def->param);
                        QByteArray cfg = m_msg->save();
                        Buffer config;
                        config = "[Title]\n" + cfg;
                        config.setWritePos(0);
                        config.getSection();
                        m_msg = (mdef->create)(&config);
                        m_msg->setContact( multiply.first() );
                        multiply.pop_front();
                        m_msg->setClient(NULL);
                        m_msg->setFlags(m_msg->getFlags() | MESSAGE_MULTIPLY);
                        if( multiply.empty() )
                            m_msg->setFlags(m_msg->getFlags() | MESSAGE_LAST);
                        send();
                        return false;
                    }
                }
                stopSend();
                bool bClose = true;
                if (CorePlugin::instance()->getContainerMode()){
                    bClose = false;
                    Command cmd;
                    cmd->id		= CmdSendClose;
                    cmd->param	= this;
                    EventCommandWidget eWidget(cmd);
                    eWidget.process();
                    QToolButton *btnClose = qobject_cast<QToolButton*>(eWidget.widget());
                    if (btnClose)
                        bClose = btnClose->isChecked();
                }
                CorePlugin::instance()->setValue("CloseSend", bClose);
                if (bClose){
                    QTimer::singleShot(0, m_userWnd, SLOT(close()));
                }else{
                    setEmptyMessage();
                    m_edit->setFont(CorePlugin::instance()->editFont);
                    m_edit->setForeground(CorePlugin::instance()->value("EditForeground").toUInt(), true);
                    m_edit->setBackground(CorePlugin::instance()->value("EditBackground").toUInt());
                }
            }
        }
        break;
    }
    default:
        break;
    }
    return false;
}

void MsgEdit::setEmptyMessage()
{
    m_edit->setPlainText(QString());
    EventMenuGetDef eMenu(MenuMessage);
    eMenu.process();
    CommandsDef *cmdsMsg = eMenu.defs();
    CommandsList itc(*cmdsMsg, true);
    CommandDef *c;
    while ((c = ++itc) != NULL){
        c->param = (void*)(m_userWnd->m_id);
        if (EventCheckCommandState(c).process()){
            Message *msg;
            CommandDef *def = CorePlugin::instance()->messageTypes.find(c->id);
            if (def == NULL)
                continue;
            MessageDef *mdef = (MessageDef*)(def->param);
            if (mdef->create == NULL)
                continue;
            msg = mdef->create(NULL);
            msg->setContact(m_userWnd->m_id);
            if (mdef->flags & MESSAGE_SILENT)
                continue;
            msg->setFlags(MESSAGE_NORAISE);
            EventOpenMessage(msg).process();
            delete msg;
            return;
        }
    }
}

void MsgEdit::changeTyping(Client *client, void *data)
{
    if (!m_bTyping)
        return;
    if (client == NULL){
        typingStop();
        return;
    }
    if (client->dataName(data) == m_typingClient)
        return;
    typingStop();
    typingStart();
}

void MsgEdit::typingStart()
{
    typingStop();
    void *data = NULL;
    Client *cl = client(data, false, false, m_userWnd->id(), m_userWnd->m_list == NULL);
    if (cl == NULL)
        return;
    Message *msg = new Message(MessageTypingStart);
    if (cl->send(msg, data)){
        m_typingClient = cl->dataName(data);
    }else{
        delete msg;
    }
}

void MsgEdit::typingStop()
{
    if (m_typingClient.isEmpty())
        return;
    Contact *contact = getContacts()->contact(m_userWnd->m_id);
    if (contact == NULL)
        return;
    ClientDataIterator it(contact->clientData);
    clientData *data;
    while ((data = ++it) != NULL){
        if (it.client()->dataName(data) == m_typingClient){
            Message *msg = new Message(MessageTypingStop);
            if (!it.client()->send(msg, data))
                delete msg;
            break;
        }
    }
    m_typingClient = QString::null;
}

void MsgEdit::editTextChanged()
{
    bool bTyping = !m_edit->isEmpty();
    if (qApp->focusWidget() != m_edit)
        bTyping = false;
    if (m_bTyping == bTyping)
        return;
    m_bTyping = bTyping;
    if (m_bTyping){
        typingStart();
    }else{
        typingStop();
    }
}

void MsgEdit::editLostFocus()
{
    if (!m_bTyping)
        return;
    typingStop();
    m_bTyping = false;
}

void MsgEdit::colorsChanged()
{
    CorePlugin::instance()->setValue("EditBackground", m_edit->background().rgb());
    CorePlugin::instance()->setValue("EditForeground", m_edit->foreground().rgb());
    EventHistoryColors().process();
}

void MsgEdit::insertSmile(const QString &id)
{
    QString img_src = QString("<img src=sim:icons/%1>").arg(id);
//    int para;
//    int index;
    QFont saveFont = m_edit->font();
    QColor saveColor = m_edit->textColor();
    // determine the current position of the cursor
    //m_edit->insert("\255", false, true, true); //FIXME
    m_edit->insertPlainText("\255");
    //m_edit->getCursorPosition(&para,&index); //FIXME
    // RTF doesn't like < and >
    QString txt = m_edit->toHtml();
    txt.replace(QRegExp("\255"),img_src);
    m_edit->setHtml(txt);
    //m_edit->setCursorPosition(para, index); //FIXME
    m_edit->setCurrentFont(saveFont);
    m_edit->setTextColor(saveColor);
}

void MsgEdit::goNext()
{
    for (list<msg_id>::iterator it = CorePlugin::instance()->unread.begin(); it != CorePlugin::instance()->unread.end(); ++it){
        if (it->contact != m_userWnd->id())
            continue;
        Message *msg = History::load(it->id, it->client, it->contact);
        if (msg == NULL)
            continue;
        EventOpenMessage(msg).process();
        delete msg;
        return;
    }
    if (CorePlugin::instance()->getContainerMode()){
        setEmptyMessage();
        return;
    }
    QTimer::singleShot(0, m_userWnd, SLOT(close()));
}

void MsgEdit::setupNext()
{
    Command cmd;
    cmd->id    = CmdNextMessage;
    cmd->param = this;
    EventCommandWidget eWidget(cmd);
    eWidget.process();
    CToolButton *btnNext = qobject_cast<CToolButton*>(eWidget.widget());
    if (btnNext == NULL)
        return;

    unsigned type  = 0;
    unsigned count = 0;
    for (list<msg_id>::iterator it = CorePlugin::instance()->unread.begin(); it != CorePlugin::instance()->unread.end(); ++it){
        if (it->contact != m_userWnd->id())
            continue;
        if (count == 0)
            type = it->type;
        count++;
    }
    QString str = i18n("&Next");
    if (count > 1)
        str += QString(" [%1]") .arg(count);

    CommandDef *def = NULL;
    def = CorePlugin::instance()->messageTypes.find(type);

    CommandDef c = btnNext->def();
    c.text_wrk = str;
    if (def)
        c.icon     = def->icon;
    if (count){
        c.flags &= ~COMMAND_DISABLED;
    }else{
        c.flags |= COMMAND_DISABLED;
    }
    btnNext->setCommand(&c);
}

void MsgEdit::editEnterPressed()
{
    Command cmd;
    cmd->id = CmdSend;
    cmd->param = this;
    EventCommandExec(cmd).process();
}

SmileLabel::SmileLabel(const QString &_id, QWidget *parent)
        : QLabel(parent), id(_id)
{
    setPixmap(Pict(_id));
    QStringList smiles = getIcons()->getSmile(_id);
    QString tip = smiles.front();
    QString name = getIcons()->getSmileName(_id);
    if ((name[0] < '0') || (name[0] > '9')){
        tip += ' ';
        tip += i18n(name);
    }
    this->setToolTip(tip);
}

void SmileLabel::mouseReleaseEvent(QMouseEvent*)
{
    emit clicked(id);
}

SmilePopup::SmilePopup(QWidget *popup)
    : QFrame(popup, Qt::Popup)
{
    setAttribute(Qt::WA_DeleteOnClose);

    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Sunken);
    setLineWidth(1);

    QSize s;
    QStringList smiles;
    getIcons()->getSmiles(smiles);
    if (smiles.empty())
        return;
    unsigned nSmiles = 0;
    QStringList::iterator it;
    for (it = smiles.begin(); it != smiles.end(); ++it)
    {
        QPixmap pict = Pict(*it);
        s = QSize(qMax(s.width(), pict.width()), qMax(s.height(), pict.height()));
        nSmiles++;
    }

    unsigned rows = 4;
    unsigned cols = (nSmiles + 3) / 4;
    if (cols > 8){
        cols = 8;
        rows = (nSmiles + 7) / cols;
    }

    QGridLayout *lay = new QGridLayout(this);
    lay->setMargin(4);
    lay->setSpacing(2);
    unsigned i = 0;
    unsigned j = 0;
    for (it = smiles.begin(); it != smiles.end(); ++it){
        QPixmap is = Pict(*it);
        if (is.isNull())
            continue;
        QWidget *w = new SmileLabel(*it, this);
        w->setMinimumSize(s);
        connect(w, SIGNAL(clicked(const QString &)), this, SLOT(labelClicked(const QString &)));
        lay->addWidget(w, i, j);
        if (++j >= cols){
            i++;
            j = 0;
        }
    }
    resize(minimumSizeHint());
}

void SmilePopup::labelClicked(const QString &id)
{
    insert(id);
    close();
}

void MsgEdit::setupMessages()
{
    Command cmd;

    cmd->id         = MessageGeneric;
    cmd->text		= I18N_NOOP("&Message");
    cmd->icon		= "message";
    cmd->menu_grp	= 0x3010;
    cmd->accel		= "Ctrl+M";
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defGeneric;
    EventCreateMessageType(cmd).process();

    cmd->id         = MessageFile;
    cmd->text		= I18N_NOOP("&File");
    cmd->icon		= "file";
    cmd->accel		= "Ctrl+F";
    cmd->menu_grp	= 0x3020;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defFile;
    EventCreateMessageType(cmd).process();

    cmd->id          = MessageUrl;
    cmd->text        = I18N_NOOP("&URL");
    cmd->icon        = "url";
    cmd->accel		 = "Ctrl+U";
    cmd->menu_grp	 = 0x3030;
    cmd->flags		 = COMMAND_DEFAULT;
    cmd->param		 = &defUrl;
    EventCreateMessageType(cmd).process();

    cmd->id			= MessageSMS;
    cmd->text		= I18N_NOOP("SMS");
    cmd->icon		= "sms";
    cmd->accel		= "Ctrl+S";
    cmd->menu_grp	= 0x3040;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defSMS;
    EventCreateMessageType(cmd).process();

    cmd->id			= MessageContacts;
    cmd->text		= I18N_NOOP("&Contact list");
    cmd->icon		= "contacts";
    cmd->accel		= "Ctrl+L";
    cmd->menu_grp	= 0x3050;
    cmd->param		= &defContacts;
    EventCreateMessageType(cmd).process();

    cmd->id			= MessageAuthRequest;
    cmd->text		= I18N_NOOP("&Authorization request");
    cmd->icon		= "auth";
    cmd->accel		= "Ctrl+Q";
    cmd->menu_grp	= 0x3060;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defAuthRequest;
    EventCreateMessageType(cmd).process();

    cmd->id			= MessageAuthGranted;
    cmd->text		= I18N_NOOP("&Grant autorization");
    cmd->icon		= "auth";
    cmd->accel		= "Ctrl+G";
    cmd->menu_grp	= 0x3070;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defAuthGranted;
    EventCreateMessageType(cmd).process();

    cmd->id			= MessageAuthRefused;
    cmd->text		= I18N_NOOP("&Refuse autorization");
    cmd->icon		= "auth";
    cmd->accel		= "Ctrl+R";
    cmd->menu_grp	= 0x3071;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defAuthRefused;
    EventCreateMessageType(cmd).process();

    cmd->id			= MessageAdded;
    cmd->text		= I18N_NOOP("Added");
    cmd->icon		= "auth";
    cmd->menu_grp	= 0;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defAdded;
    EventCreateMessageType(cmd).process();

    cmd->id			= MessageRemoved;
    cmd->text		= I18N_NOOP("Removed");
    cmd->icon		= "auth";
    cmd->menu_grp	= 0;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defRemoved;
    EventCreateMessageType(cmd).process();

    cmd->id			= MessageStatus;
    cmd->text		= I18N_NOOP("Status");
    cmd->icon		= QString::null;
    cmd->menu_grp	= 0;
    cmd->flags		= COMMAND_DEFAULT;
    cmd->param		= &defStatus;
    EventCreateMessageType(cmd).process();
}

