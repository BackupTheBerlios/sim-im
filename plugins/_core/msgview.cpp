/***************************************************************************
                          msgview.cpp  -  description
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

#include "icons.h"
#include "html.h"
#include "unquot.h"
#include "xsl.h"
#include "contacts/clientdataiterator.h"
#include "contacts/contact.h"
#include "contacts/client.h"

#include "msgview.h"
#include "core.h"
#include "history.h"

#include <QStringList>
#include <QTimer>
#include <QDateTime>
#include <QMenu>
#include <QScrollBar>
#include <QTextBlock>
#include <QContextMenuEvent>
#include <QTextDocumentFragment>

using namespace std;
using namespace SIM;

static char MSG_ANCHOR[] = "<a name=\"m://";
static char MSG_BEGIN[]  = "<a name=\"b\"/>";

class ViewParser : public HTMLParser
{
public:
    ViewParser(bool bIgnoreColors, bool bUseSmiles);
    QString parse(const QString &str);
protected:
    QString res;
    bool m_bIgnoreColors;
    bool m_bUseSmiles;
    bool m_bInLink;
    bool m_bInHead;
    bool m_bInParagraph;
    bool m_bParagraphEmpty;
    bool m_bFirst;
    bool m_bSpan;
    // Marks the position in 'res' where " DIR=\"whatever\"" should be inserted,
    // if the paragraph is DIR-less and we determine the DIR later on.
    unsigned m_paraDirInsertionPos;
    enum {
        DirAuto, // Initial BiDi dir when not explicitly specified.
        // Real dir will be determined from the first
        // strong BiDi character.
        DirLTR,
        DirRTL,
        DirUnknown
    } m_paragraphDir;
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);
};

/*
   This parser is run on the output of the 'history XSL'.
   The text which the XSL process output should generally be HTML,
   but may contain the following special tags:
 
    <prepend>...</prepend>
    Strips the PREPEND tags and prepends their contents to the beginning
    of the next paragraph. Useful to make sure chat prefixes are prepended
    to the first paragraph of a multi-paragraph message (instead of residing
    on a new paragraph).
*/

class XslOutputParser : public HTMLParser
{
public:
    XslOutputParser();
    QString parse(const QString &str);

protected:
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);

protected:
    QString res;
    bool m_bInPrepend;
    QString m_sPrepend;
};

XslOutputParser::XslOutputParser()
        : m_bInPrepend(false)
{
}

QString XslOutputParser::parse(const QString &str)
{
    res = QString::null;
    HTMLParser::parse(str);
    if (!m_sPrepend.isEmpty())
        res = m_sPrepend + res;
    return res;
}

void XslOutputParser::text(const QString& text)
{
    if (m_bInPrepend)
        m_sPrepend += quoteString(text);
    else
        res += quoteString(text);
}

void XslOutputParser::tag_start(const QString &tag, const list<QString> &attrs)
{
    QString ltag = tag.toLower();

    if (ltag == "prepend")
    {
        m_bInPrepend = true;
        return;
    }

    QString tagText;
    tagText += '<';
    tagText += tag;
    for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
        QString name = *it;
        ++it;
        QString value = *it;
        tagText += ' ';
        tagText += name;
        if (!value.isEmpty()){
            tagText += "=\"";
            tagText += value;
            tagText += '\"';
        }
    }
    tagText += '>';

    if (m_bInPrepend)
    {
        m_sPrepend += tagText;
    }
    else
    {
        res += tagText;

        // It's time to prepend whatever we've got in m_sPrepend
        // to the start of a paragraph.
        if ((ltag == "p") && !m_sPrepend.isEmpty())
        {
            res += m_sPrepend;
            m_sPrepend = QString::null;
        }
    }
}

void XslOutputParser::tag_end(const QString &tag)
{
    QString ltag = tag.toLower();

    if (ltag == "prepend")
    {
        m_bInPrepend = false;
        return;
    }

    QString tagText;
    tagText += "</";
    tagText += tag;
    tagText += '>';

    if (m_bInPrepend)
        m_sPrepend += tagText;
    else
        res += tagText;
}

MsgViewBase::MsgViewBase(QWidget *parent, const char *name, unsigned id)
        : TextShow      (parent, name)
        , m_id          (id)
        , m_popupPos    (QPoint(0, 0))
        , m_nSelection  (0)
        , xsl           (NULL)
{
    // Disable top and bottom margins for P tags. This will make sure
    // paragraphs have no more spacing than regular lines, thus matching
    // RTF's defaut look for paragraphs.
    document()->setDefaultStyleSheet("p { margin-top: 0; margin-bottom: 0; }");

    setColors();
    setFont(CorePlugin::instance()->editFont);
    setContextMenuPolicy( Qt::DefaultContextMenu );
}

MsgViewBase::~MsgViewBase()
{
    if (xsl)
        delete xsl;
}

void MsgViewBase::setXSL(XSL *n_xsl)
{
    if (xsl)
        delete xsl;
    xsl = n_xsl;
}

void MsgViewBase::setSelect(const QString &str)
{
    m_nSelection = 0;
    m_selectStr = str;
}

void MsgViewBase::update()
{
    if (m_updated.empty())
        return;
/*
    unsigned i;
    for (i = 0; i < (unsigned)paragraphs(); i++){
        QString s = text(i);
        int n = s.indexOf(MSG_ANCHOR);
        if (n < 0)
            continue;
        s = s.mid(n + strlen(MSG_ANCHOR));
        n = s.indexOf('\"');
        if (n < 0)
            continue;
        QString client;
        unsigned id = messageId(s.left(n), client);
        list<Msg_Id>::iterator it;
        for (it = m_updated.begin(); it != m_updated.end(); ++it){
            if ((it->id == id) && (it->client == client))
                break;
        }
        if (it != m_updated.end())
            break;
    }
    m_updated.clear();
    if (i >= (unsigned)paragraphs())
        return;
    int x = contentsX();
    int y = contentsY();
    viewport()->setUpdatesEnabled(false);

    unsigned start = i;
    list<Msg_Id> msgs;
    for (; i < (unsigned)paragraphs(); i++){
        QString s = text(i);
        int n = s.indexOf(MSG_ANCHOR);
        if (n < 0)
            continue;
        s = s.mid(n + strlen(MSG_ANCHOR));
        n = s.indexOf('\"');
        if (n < 0)
            continue;
        QString client;
        unsigned id = messageId(s.left(n), client);
        list<Msg_Id>::iterator it;
        for (it = msgs.begin(); it != msgs.end(); ++it){
            if ((it->id == id) && (it->client == client))
                break;
        }
        if (it != msgs.end())
            continue;
        Msg_Id m_id;
        m_id.id     = id;
        m_id.client = client;
        msgs.push_back(m_id);
    }
    int paraFrom, indexFrom;
    int paraTo, indexTo;
    getSelection(&paraFrom, &indexFrom, &paraTo, &indexTo);
    setReadOnly(false);
    setSelection(start, 0, paragraphs() - 1, 0xFFFF, 0);
    removeSelectedText();
    setReadOnly(true);
    QString text;
    for (list<Msg_Id>::iterator it = msgs.begin(); it != msgs.end(); ++it){
        Message *msg = History::load(it->id, it->client, m_id);
        if (msg == NULL)
            continue;
        bool bUnread = false;
        for (list<msg_id>::iterator itu = CorePlugin::instance()->unread.begin(); itu != CorePlugin::instance()->unread.end(); ++itu){
            msg_id &m = (*itu);
            if ((m.contact == msg->contact()) &&
                    (m.id == msg->id()) &&
                    (m.client == msg->client())){
                bUnread = true;
                break;
            }
        }
        text += messageText(msg, bUnread);
        delete msg;
    }
    viewport()->setUpdatesEnabled(true);
    append(text);   //<= here occurred a crash
    if (!CorePlugin::instance()->getOwnColors())
        setBackground(i);
    if ((paraFrom != paraTo) || (indexFrom != indexTo))
        setSelection(paraFrom, indexFrom, paraTo, indexTo, 0);
    TextShow::sync();
    setContentsPos(x, y);
    viewport()->repaint();
*/
}

//ToDo: Rewrite this ugly slow Function which is slowing down the History loading. 
QString MsgViewBase::messageText(Message *msg, bool bUnread)
{
    QString options;
    QString info;
    QString status;

    QString icon = "message";
    const CommandDef *def = CorePlugin::instance()->messageTypes.find(msg->type());
    if (def)
        icon = def->icon;
    bool bDirection = false;
    if (msg->type() == MessageStatus){
        icon = "empty";
        StatusMessage *sm = static_cast<StatusMessage*>(msg);
        Client *client = NULL;
        QString clientStr = msg->client();
        int n = clientStr.lastIndexOf('.');
        if (n >= 0){
            clientStr = clientStr.left(n);
        }else{
            clientStr.clear();
        }
        if (!clientStr.isEmpty()){
            for (unsigned i = 0; i < getContacts()->nClients(); i++){
                QString n = getContacts()->getClient(i)->name();
                if (n.startsWith(clientStr)){
                    client = getContacts()->getClient(i);
                    break;
                }
            }
        }
        if ((client == NULL) && getContacts()->nClients())
            client = getContacts()->getClient(0);
        if(client) {
            for (def = client->protocol()->statusList(); !def->text.isEmpty(); def++){
                if (def->id == sm->getStatus()){
                    icon = def->icon;
                    status = i18n(def->text);
                    break;
                }
            }
        }
        options += " direction=\"2\"";
        bDirection = true;
    }else{
        MessageDef *m_def = (MessageDef*)(def->param);
        if (m_def->flags & MESSAGE_INFO){
            options += " direction=\"2\"";
            bDirection = true;
        }
    }
    info = QString("<icon>%1</icon>") .arg(icon);

    QString contactName;
    if (msg->getFlags() & MESSAGE_RECEIVED){
        if (!bDirection)
            options += " direction=\"1\"";
        Contact *contact = getContacts()->contact(msg->contact());
        if (contact){
            contactName = contact->getName();
            if (contactName.isEmpty()){
                Client *client = NULL;
                ClientDataIterator it(contact->clientData);
                void *data;
                while ((data = ++it) != NULL){
                    if (it.client()->dataName(data) == msg->client()){
                        client = it.client();
                        break;
                    }
                }
            }
        }
        if (!bUnread){
            for (list<msg_id>::iterator it = CorePlugin::instance()->unread.begin(); it != CorePlugin::instance()->unread.end(); ++it){
                msg_id &m = (*it);
                if ((m.id == msg->id()) &&
                        (m.contact == msg->contact()) &&
                        (m.client == msg->client())){
                    bUnread = true;
                    break;
                }
            }
        }
        if (bUnread)
            options += " unread=\"1\"";
    }else{
        if (!bDirection)
            options += " direction=\"0\"";
        contactName = getContacts()->owner()->getName();
    }
    if (contactName.isEmpty())
        contactName = "???";
    info += QString("<from>%1</from>") .arg(quoteString(contactName));
    QString id = QString::number(msg->id());
    id += ',';
    // <hack>
    // Terrible hack to set message bgcolor. We prefer to insert the entire history
    // in one chunk (since it's more efficient and causes less redraws), and there's
    // no way to set block's background color directly in Qt's HTML), so we make a note
    // of it in the HTML and set it retroactively in setBackground.
    if (!CorePlugin::instance()->value("OwnColors").toBool() && (msg->getBackground() != 0xFFFFFFFF) && (msg->getForeground() != msg->getBackground()))
        id += QString::number(msg->getBackground());
    // </hack>
    QString client_str = msg->client();
    if (!client_str.isEmpty()){
        id += ',';
        id += quoteString(client_str);
    }
    if (m_cut.size()){
        id += ',';
        id += QString::number(m_cut.size());
    }
    info += "<id>";
    info += id;
    info += "</id>";


    QString icons;
    if (msg->getFlags() & MESSAGE_SECURE)
        options += " encrypted=\"1\"";
    if (msg->getFlags() & MESSAGE_URGENT)
        options += " urgent=\"1\"";
    if (msg->getFlags() & MESSAGE_LIST)
        options += " list=\"1\"";

    QString s;
    QDateTime t;
    t.setTime_t(msg->getTime());
    info += s.sprintf("<time><date>%%1</date><hour>%02u</hour><minute>%02u</minute><second>%02u</second></time>",
                      t.time().hour(), t.time().minute(), t.time().second()) .arg(formatDate(QDateTime::fromTime_t(msg->getTime()).date()));

    s = "<?xml version=\"1.0\"?><message";
    s += options;
    s += '>';
    s += info;

    QString msgText;
    if (msg->type() != MessageStatus){
        msgText = msg->presentation();
        if (msgText.isEmpty()){
            unsigned type = msg->baseType();
            CommandDef *cmd = CorePlugin::instance()->messageTypes.find(type);
            if (cmd){
                MessageDef *def = (MessageDef*)(cmd->param);
                msgText = i18n(def->singular, def->plural, 1);
                int n = msgText.indexOf("1 ");
                if (n == 0){
                    msgText = msgText.mid(2);
                }else if (n > 0){
                    msgText = msgText.left(n);
                }
                msgText = QString("<p>") + msgText + "</p>";
            }
            QString text = msg->getRichText();
            msgText += text;
        }
    }else{
        msgText = status;
    }
    EventAddHyperlinks e(msgText);
    e.process();
    ViewParser parser(CorePlugin::instance()->value("OwnColors").toBool(), CorePlugin::instance()->value("UseSmiles").toBool());
    msgText = parser.parse(e.text());
    s += "<body";
    if (!CorePlugin::instance()->value("OwnColors").toBool() && (msg->getForeground() != 0xFFFFFFFF) && (msg->getForeground() != msg->getBackground()))
    {
        s += " fgcolor=\"#";
        s += QString::number(msg->getForeground(), 16).rightJustified(6, '0');
        s += '\"';
    }

    // Some bright day might come when one could specify background color from inside Qt's richtext.
    // Meanwhile, this is useless:
    if ((msg->getBackground() != 0xFFFFFFFF) && (msg->getForeground() != msg->getBackground()))
    {
        s += " bgcolor=\"#";
        s += QString::number(msg->getBackground(), 16).rightJustified(6, '0');
        s += '\"';
    }
    s += '>';

    // We pass the rich text quoted, since we're not sure of its' XML validity.
    // The XSL engine should copy it as-is (using xsl:value-of with disable-output-escaping="yes").
    s += quoteString(QString(MSG_BEGIN) + msgText);

    s += "</body>";
    s += "</message>";
    XSL *p = xsl;
    if (p == NULL)
        p = CorePlugin::instance()->historyXSL;
    QString res = p->process(s);

    XslOutputParser outParser;
    res = outParser.parse(res);

    QString anchor = MSG_ANCHOR;
    anchor += id;
    anchor += "\"/>";
    res = "<p>" + anchor + res + "</p>";
    return res;
}

void MsgViewBase::setSource(const QUrl& url)
{
    setSource(url.toString());
}

void MsgViewBase::setSource(const QString &url)
{
    QString proto;
    int n = url.indexOf(':');
    if (n >= 0)
        proto = url.left(n);
    if (proto != "msg"){
        TextShow::setSource(url);
        return;
    }
    QString id = url.mid(proto.length() + 3);
    unsigned msg_id = getToken(id, ',').toULong();
    getToken(id, ',');
    id = getToken(id, '/');
    QString client = SIM::unquoteString(id);
    if (client.isEmpty())
        client = QString::number(m_id);
    Message *msg = History::load(msg_id, client, m_id);
    if (msg){
        EventOpenMessage(msg).process();
        delete msg;
    }
}

// <hack>
// We have to use this function since Qt has no tag to set background color per-paragraph
// from within HTML. See matching hack in MsgViewBase::messageText.
void MsgViewBase::setBackground(unsigned /*n*/)
{
/*
    QColor bgcolor;
    bool bInMsg = false;
    bool bSet   = false;

    QString sAnchor = QString::fromLatin1(MSG_ANCHOR),
                      sBegin = QString::fromLatin1(MSG_BEGIN);

    int i;
    for (i = n; i >= 0; i--){
        QString s = text(i);
        if (s.indexOf(sAnchor) >= 0)
            break;
    }
    for (; i < paragraphs(); i++){
        QString s = text(i);
        int anchorPos = s.indexOf(sAnchor);
        if (anchorPos >= 0)
        {
            bInMsg = false;
            bSet   = false;

            // This code could be a bit faster by making assumptions.
            // However, I prefer to be correct HTML-parser-wise.

            int idStart = anchorPos + sAnchor.length();
            int idEnd = s.indexOf('\"', idStart);
            if ((idStart >= 0) && (idEnd >= 0))
            {
                QString id = s.mid(idStart, idEnd - idStart);

                // Parse the message id (msgId,backgroundColor,...)
                int bgcolorStart = id.indexOf(',');
                if (bgcolorStart >= 0)
                {
                    QString sBgcolor = id.mid(bgcolorStart + 1);
                    int bgcolorEnd = sBgcolor.indexOf(',');
                    if (bgcolorEnd > 0)
                        sBgcolor = sBgcolor.left(bgcolorEnd);
                    if (!sBgcolor.isEmpty())
                        bgcolor = QColor(sBgcolor.toULong(&bSet));
                }
            }
        }
        if (s.indexOf(sBegin) >= 0)
            bInMsg = true;

        if (bInMsg && bSet){
            setParagraphBackgroundColor(i, bgcolor);
        }else{
            clearParagraphBackground(i);
        }
    }
*/
}
// </hack>

void MsgViewBase::addMessage(Message *msg, bool bUnread, bool /*bSync*/)
{
    unsigned n = document()->blockCount();
    if (n > 0)
        n--;
    append(messageText(msg, bUnread));
    if (!CorePlugin::instance()->value("OwnColors").toBool())
        setBackground(n);
}

bool MsgViewBase::findMessage(Message *msg)
{
    QTextBlock block = document()->firstBlock();
    while( block.isValid() ) {
        QTextCursor cursor( block );
        cursor.select( QTextCursor::BlockUnderCursor );
        QTextDocumentFragment fragment( cursor );
        QString s = fragment.toHtml();
        int n = s.indexOf(MSG_ANCHOR);
        if (n >= 0) {
            s = s.mid(n + strlen(MSG_ANCHOR));
            n = s.indexOf('\"');
            if (n >= 0) {
                QString client;
                if ((messageId(s.left(n), client) == msg->id()) && (client == msg->client())) {
                    return true;
                }
            }
        }
        block = block.next();
    }

    return false;
}

void MsgViewBase::setColors()
{
    TextShow::setBackground(CorePlugin::instance()->value("EditBackground").toUInt());
    TextShow::setForeground(CorePlugin::instance()->value("EditForeground").toUInt());
}

unsigned MsgViewBase::messageId(const QString &_s, QString &client)
{
    QString s(_s);
    unsigned id = getToken(s, ',').toULong();
    getToken(s, ',');
    client = getToken(s, ',');
    if (id >= 0x80000000)
        return id;
    for (unsigned cut_id = s.toUInt(); cut_id < m_cut.size(); cut_id++){
        if (m_cut[cut_id].client != client)
            continue;
        if (id < m_cut[cut_id].from)
            continue;
        id -= m_cut[cut_id].size;
    }
    return id;
}

void MsgViewBase::reload()
{
/*
    QString t;
    vector<Msg_Id> msgs;
    unsigned i;
    for (i = 0; i < (unsigned)paragraphs(); i++){
        QString s = text(i);
        int n = s.indexOf(MSG_ANCHOR);
        if (n < 0)
            continue;
        s = s.mid(n + strlen(MSG_ANCHOR));
        n = s.indexOf('\"');
        if (n < 0)
            continue;
        Msg_Id id;
        id.id = messageId(s.left(n), id.client);
        unsigned nn;
        for (nn = 0; nn < msgs.size(); nn++){
            if ((msgs[nn].id == id.id) && (msgs[nn].client == id.client))
                break;
        }
        if (nn >= msgs.size())
            msgs.push_back(id);
    }
    for (i = 0; i < msgs.size(); i++){
        Message *msg = History::load(msgs[i].id, msgs[i].client, m_id);
        if (msg == NULL)
            continue;
        t += messageText(msg, false);
        delete msg;
    }
    QPoint p = QPoint(0, height());
    p = mapToGlobal(p);
    p = viewport()->mapFromGlobal(p);
    int x, y;
    viewportToContents(p.x(), p.y(), x, y);
    int para;
    int pos = charAt(QPoint(x, y), &para);
    setText(t);
    if (!CorePlugin::instance()->getOwnColors())
        setBackground(0);
    if (pos == -1){
        scrollToBottom();
    }else{
        setCursorPosition(para, pos);
        ensureCursorVisible();
    }
*/
}

bool MsgViewBase::processEvent(Event *e)
{
    if ((e->type() == eEventRewriteMessage) || (e->type() == eEventMessageRead)){
        /*
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        if (msg->contact() != m_id)
            return false;
        unsigned i;
        for (i = 0; i < (unsigned)paragraphs(); i++){
            QString s = text(i);
            int n = s.indexOf(MSG_ANCHOR);
            if (n < 0)
                continue;
            s = s.mid(n + strlen(MSG_ANCHOR));
            n = s.indexOf('\"');
            if (n < 0)
                continue;
            QString client;
            if ((messageId(s.left(n), client) == msg->id()) && (client == msg->client()))
                break;
        }
        if (i >= (unsigned)paragraphs())
            return false;
        Msg_Id id;
        id.id     = msg->id();
        id.client = msg->client();
        m_updated.push_back(id);
        QTimer::singleShot(0, this, SLOT(update()));
        */
        return false;
    }
    if (e->type() == eEventCutHistory){
        /*
        EventCutHistory *ech = static_cast<EventCutHistory*>(e);
        CutHistory *ch = ech->cut();
        if (ch->contact != m_id)
            return false;

        bool bDelete = false;
        vector<unsigned> start_pos;
        vector<unsigned> end_pos;
        for (unsigned i = 0; i < (unsigned)paragraphs(); i++){
            QString s = text(i);
            int n = s.indexOf(MSG_ANCHOR);
            if (n < 0)
                continue;
            s = s.mid(n + strlen(MSG_ANCHOR));
            n = s.indexOf('\"');
            if (n < 0)
                continue;
            QString client;
            unsigned id = messageId(s.left(n), client);
            if ((client == ch->client) && (id >= ch->from) && (id < ch->from + ch->size)){
                if (!bDelete){
                    bDelete = true;
                    start_pos.push_back(i);
                }
            }else{
                if (bDelete){
                    bDelete = false;
                    end_pos.push_back(i);
                }
            }
        }
        if (bDelete)
            end_pos.push_back(paragraphs());
        if (start_pos.size()){
            int paraFrom, indexFrom;
            int paraTo, indexTo;
            getSelection(&paraFrom, &indexFrom, &paraTo, &indexTo);
            QPoint p = QPoint(0, 0);
            p = mapToGlobal(p);
            p = viewport()->mapFromGlobal(p);
            int x, y;
            viewportToContents(p.x(), p.y(), x, y);
            int para;
            int pos = charAt(QPoint(x, y), &para);
            setReadOnly(false);
            for (unsigned i = 0; i < start_pos.size(); i++){
                setSelection(start_pos[i], 0, end_pos[i], 0, 0);
                removeSelectedText();
                if ((unsigned)pos >= start_pos[i])
                    pos = end_pos[i] - start_pos[i];
            }
            if ((paraFrom == -1) && (paraTo == -1)){
                if (pos == -1){
                    scrollToBottom();
                }else{
                    setCursorPosition(para, pos);
                    ensureCursorVisible();
                }
            }else{
                setSelection(paraFrom, indexFrom, paraTo, indexTo, 0);
            }
            setReadOnly(true);
            repaint();
        }
        m_cut.push_back(*ch);
        */
        return false;
    }
    if (e->type() == eEventMessageDeleted){
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        if (msg->contact() != m_id)
            return false;
        /*
        for (unsigned i = 0; i < (unsigned)paragraphs(); i++){
            unsigned j;
            QString s = text(i);
            int n = s.indexOf(MSG_ANCHOR);
            if (n < 0)
                continue;
            s = s.mid(n + strlen(MSG_ANCHOR));
            n = s.indexOf('\"');
            if (n < 0)
                continue;
            QString client;
            if ((messageId(s.left(n), client) != msg->id()) || (client != msg->client()))
                continue;

            for (j = i + 1; j < (unsigned)paragraphs(); j++){
                QString s = text(j);
                int n = s.indexOf(MSG_ANCHOR);
                if (n < 0)
                    continue;
                s = s.mid(n + strlen(MSG_ANCHOR));
                n = s.indexOf('\"');
                if (n < 0)
                    continue;
                QString client;
                if ((messageId(s.left(n), client) != msg->id()) || (client != msg->client()))
                    break;
            }
            int paraFrom, indexFrom;
            int paraTo, indexTo;
            getSelection(&paraFrom, &indexFrom, &paraTo, &indexTo);
            unsigned pos = 0xFFFF;
            if (j == (unsigned)paragraphs()){
                j++;
                pos = 0;
            }
            setSelection(i, 0, j - 1, pos, 0);
            setReadOnly(false);
            removeSelectedText();
            setReadOnly(true);
            if ((paraFrom == -1) && (paraTo == -1)){
                scrollToBottom();
            }else{
                setSelection(paraFrom, indexFrom, paraTo, indexTo, 0);
            }
            break;
        }
        */
        return false;
    }
    if (e->type() == eEventHistoryConfig){
        EventHistoryConfig *ehc = static_cast<EventHistoryConfig*>(e);
        unsigned long id = ehc->id();
        if (id && (id != m_id))
            return false;
        reload();
    } else
    if (e->type() == eEventHistoryColors) {
        setColors();
    } else
    if (e->type() == eEventCheckCommandState){
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if ((cmd->param != this) || (cmd->menu_id != MenuMsgView))
            return false;
        Message *msg;
        switch (cmd->id){
        case CmdCopy:
            cmd->flags &= ~(COMMAND_DISABLED | COMMAND_CHECKED);
            if (!textCursor().hasSelection())
                cmd->flags |= COMMAND_DISABLED;
            return true;
        case CmdMsgOpen:
            msg = currentMessage();
            if (msg){
                unsigned type = msg->baseType();
                delete msg;
                CommandDef *def = CorePlugin::instance()->messageTypes.find(type);
                if (def == NULL)
                    return false;
                cmd->icon = def->icon;
                cmd->flags &= ~COMMAND_CHECKED;
                return true;
            }
            return false;
        case CmdMsgSpecial:
            msg = currentMessage();
            if (msg){
                EventMenuGetDef eMenu(MenuMsgCommand);
                eMenu.process();
                CommandsDef *cmdsMsg = eMenu.defs();

                unsigned n = 0;
                MessageDef *mdef = NULL;
                unsigned type = msg->baseType();
                const CommandDef *cmdsSpecial = NULL;
                CommandDef *msgCmd = CorePlugin::instance()->messageTypes.find(type);
                if (msgCmd)
                    mdef = (MessageDef*)(msgCmd->param);

                if (mdef){
                    if (msg->getFlags() & MESSAGE_RECEIVED){
                        cmdsSpecial = mdef->cmdReceived;
                    }else{
                        cmdsSpecial = mdef->cmdSent;
                    }
                    if (cmdsSpecial)
                        for (const CommandDef *d = cmdsSpecial; !d->text.isEmpty(); d++)
                            n++;
                }

                {
                    CommandsList it(*cmdsMsg, true);
                    while (++it)
                        n++;
                }
                if (n == 0)
                    return false;

                n++;
                CommandDef *cmds = new CommandDef[n];
                n = 0;
                if (cmdsSpecial){
                    for (const CommandDef *d = cmdsSpecial; !d->text.isEmpty(); d++){
                        cmds[n] = *d;
                        cmds[n].id = CmdMsgSpecial + n;
                        cmds[n].flags = COMMAND_DEFAULT;
                        n++;
                    }
                }
                CommandDef *c;
                CommandsList it(*cmdsMsg, true);
                while ((c = ++it) != NULL){
                    CommandDef cmd = *c;
                    cmd.menu_id = MenuMsgCommand;
                    cmd.param   = msg;
                    if (!EventCheckCommandState(&cmd).process())
                        continue;
                    cmd.flags &= ~COMMAND_CHECK_STATE;
                    cmds[n++] = cmd;
                }
                cmd->param = cmds;
                cmd->flags |= COMMAND_RECURSIVE;
                delete msg;
                return true;
            }
            return false;
        }
    } else
    if (e->type() == eEventCommandExec){
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if ((cmd->param != this) || (cmd->menu_id != MenuMsgView))
            return false;
        Message *msg;
        switch (cmd->id){
        case CmdCutHistory:
            msg = currentMessage();
            if (msg){
                History::cut(msg, 0, 0);
                delete msg;
                return true;
            }
            return false;
        case CmdDeleteMessage:
            msg = currentMessage();
            if (msg){
                History::del(msg);
                delete msg;
                return true;
            }
            return false;
        case CmdCopy:
            copy();
            return true;
        case CmdMsgOpen:
            msg = currentMessage();
            if (msg){
                msg->setFlags(msg->getFlags() | MESSAGE_OPEN);
                EventOpenMessage(msg).process();
                delete msg;
                return true;
            }
            return false;
        default:
            msg = currentMessage();
            if (msg){
                if (cmd->id >= CmdMsgSpecial){
                    MessageDef *mdef = NULL;
                    unsigned type = msg->baseType();
                    CommandDef *msgCmd = CorePlugin::instance()->messageTypes.find(type);
                    if (msgCmd)
                        mdef = (MessageDef*)(msgCmd->param);
                    const CommandDef *cmds = NULL;
                    if (mdef){
                        if (msg->getFlags() & MESSAGE_RECEIVED){
                            cmds = mdef->cmdReceived;
                        }else{
                            cmds = mdef->cmdSent;
                        }
                    }

                    if (cmds){
                        unsigned n = cmd->id - CmdMsgSpecial;
                        for (const CommandDef *d = cmds; !d->text.isEmpty(); d++){
                            if (n-- == 0){
                                CommandDef cmd = *d;
                                cmd.param = msg;
                                cmd.menu_id = 0;
                                EventCommandExec(&cmd).process();
                                return true;
                            }
                        }
                    }
                }
                Command c;
                c->id = cmd->id;
                c->menu_id = MenuMsgCommand;
                c->param = msg;
                EventCommandExec e(c);
                bool res = e.process();
                delete msg;
                return res;
            }
            return false;
        }
    }
    return false;
}

Message *MsgViewBase::currentMessage()
{
    QTextCursor cursor = cursorForPosition( m_popupPos );
    QTextBlock block = cursor.block();
    if( !block.isValid() )
        return NULL;
    while( block.isValid() ) {
        QTextCursor cursor( block );
        cursor.select( QTextCursor::BlockUnderCursor );
        QTextDocumentFragment fragment( cursor );
        QString s = fragment.toHtml();
        int n = s.indexOf(MSG_ANCHOR);
        if (n >= 0) {
            s = s.mid(n + strlen(MSG_ANCHOR));
            n = s.indexOf('\"');
            if (n >= 0) {
                QString client;
                unsigned id = messageId(s.left(n), client);
                Message *msg = History::load(id, client, m_id);
                if (msg)
                    return msg;
            }
        }
        block = block.previous();
    }
    return NULL;
}

void MsgViewBase::contextMenuEvent( QContextMenuEvent *event )
{
    m_popupPos = event->pos();
    Command cmd;
    cmd->popup_id	= MenuMsgView;
    cmd->param		= this;
    cmd->flags		= COMMAND_NEW_POPUP;
    EventMenuGet e(cmd);
    e.process();
    QMenu *pMenu = e.menu();
    pMenu->exec( event->globalPos() );
    delete pMenu;
}

MsgView::MsgView(QWidget *parent, unsigned id)
        : MsgViewBase(parent, NULL, id)
{
    int nCopy = CorePlugin::instance()->value("CopyMessages").toUInt();
    unsigned nUnread = 0;
    for (list<msg_id>::iterator it = CorePlugin::instance()->unread.begin(); it != CorePlugin::instance()->unread.end(); ++it){
        msg_id &m = (*it);
        if (m.contact == m_id)
            nUnread++;
    }
    if (nCopy || nUnread){
        QString t = toHtml();
        HistoryIterator it(m_id);
        it.end();
        while ((nCopy > 0) || nUnread){
            Message *msg = --it;
            if (msg == NULL)
                break;
            t = messageText(msg, false) + t;
            nCopy--;
            if (nUnread == 0)
                continue;
            for (list<msg_id>::iterator it = CorePlugin::instance()->unread.begin(); it != CorePlugin::instance()->unread.end(); ++it){
                msg_id &m = (*it);
                if ((m.contact == msg->contact()) &&
                        (m.id == msg->id()) &&
                        (m.client == msg->client())){
                    nUnread--;
                    break;
                }
            }
        }
        setHtml(t);
        if (!CorePlugin::instance()->value("OwnColors").toBool())
            setBackground(0);
    }
    QScrollBar *sbar = verticalScrollBar();
    if( NULL != sbar ) {
        sbar->setValue( sbar->maximum() );
    }
    QTimer::singleShot(0, this, SLOT(init()));
}

MsgView::~MsgView()
{
}

void MsgView::init()
{
    QScrollBar *sbar = verticalScrollBar();
    if( NULL != sbar ) {
        sbar->setValue( sbar->maximum() );
    }
}

bool MsgView::processEvent(Event *e)
{
    if ((e->type() == eEventSent) || (e->type() == eEventMessageReceived)){
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        if (msg->contact() != m_id)
            return false;
        if (msg->getFlags() & MESSAGE_NOVIEW)
            return false;
        bool bAdd = true;
        if (msg->type() == MessageStatus){
            bAdd = false;
            Contact *contact = getContacts()->contact(msg->contact());
            if (contact){
				SIM::PropertyHubPtr data = contact->getUserData("_core");
                if (!data.isNull() && data->value("LogStatus").toBool() != NEW_MSG_NOOPEN)
                    bAdd = true;
            }
        }
        if (bAdd && (e->type() == eEventMessageReceived)){
            Contact *contact = getContacts()->contact(msg->contact());
            if (contact){
				SIM::PropertyHubPtr data = contact->getUserData("_core", true);
                if (data->value("OpenNewMessage").toUInt() != NEW_MSG_NOOPEN)
                    bAdd = false;
            }
        }
        if (bAdd){
            addMessage(msg);
            if (!textCursor().hasSelection()) {
                QScrollBar *sbar = verticalScrollBar();
                if( NULL != sbar ) {
                    sbar->setValue( sbar->maximum() );
                }
            }
        }
    }
    return MsgViewBase::processEvent(e);
}

ViewParser::ViewParser(bool bIgnoreColors, bool bUseSmiles)
{
    m_bIgnoreColors = bIgnoreColors;
    m_bUseSmiles    = bUseSmiles;
    m_bInLink       = false;
    m_bInHead       = false;
    m_bInParagraph  = false;
    m_bParagraphEmpty = true;
    m_bFirst     	= true;
    m_bSpan			= false;
}

QString ViewParser::parse(const QString &str)
{
    res = QString::null;
    HTMLParser::parse(str);
    return res;
}

void ViewParser::text(const QString &text)
{
    if (text.isEmpty())
        return;
        
    if (m_bInParagraph)
       m_bParagraphEmpty = false;

    if (m_bInParagraph && (m_paragraphDir == DirAuto))
    {
        /* text isn't (unicode)-NULL terminated so we can't check for c->isNull! */
        for(unsigned int i = 0; ((i < (unsigned)text.length()) && (m_paragraphDir == DirAuto)); i++)
        {
            const QChar c = text.unicode()[i];
            // Note: Qt expects ltr/rtl to be lower-case.
            switch(c.direction())
            {
            case QChar::DirL:
                res.insert(m_paraDirInsertionPos, " dir=\"ltr\"");
                m_paragraphDir = DirLTR;
                break;
            case QChar::DirR:
                res.insert(m_paraDirInsertionPos, " dir=\"rtl\"");
                m_paragraphDir = DirRTL;
                break;
            default: // avoid gcc warning
                break;
            }
        }
    }

    if (!m_bUseSmiles || m_bInLink){
        res += quoteString(text);
        return;
    }
    m_bFirst = false;
    if (m_bUseSmiles){
        QString r = getIcons()->parseSmiles(text);
        res += r;
    }else{
        res += quoteString(text);
    }
}

static const char *def_smiles[] =
    {
        ":-)",
        ":-0",
        ":-|",
        ":-/",
        ":-(",
        ":-{}",
        ":*)",
        ":'-(",
        ";-)",
        ":-@",
        ":-\")",
        ":-X",
        ":-P",
        "8-)",
        "O:-)",
        ":-D",
        "*ANNOYED*",
        "*DISGUSTED*",
        "*DROOLING*",
        "*GIGGLING*",
        "*JOKINGLY*",
        "*SHOCKED*",
        "*WHINING*",
        "*SURPRISED*",
        "*SURPRISED*",
        "*IN LOVE*"
    };

void ViewParser::tag_start(const QString &tag, const list<QString> &attrs)
{
    // the tag that will be actually written out
    QString oTag = tag;

    if (m_bInHead)
        return;

    QString style;

    if (tag == "img"){
        QString src;
        for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
            QString name = it->toLower();
            ++it;
            QString value = *it;
            if (name == "src"){
                src = value;
                break;
            }
        }
        if (src.startsWith("icon:smile")){
            bool bOK;
            unsigned nSmile = src.mid(10).toUInt(&bOK, 16);
            if (bOK && (nSmile < 26)){
                QString s = def_smiles[nSmile];
                res += getIcons()->parseSmiles(s);
                return;
            }
        }
    }else if (tag == "a"){
        m_bInLink = true;
    }else if (tag == "p"){
        m_bInParagraph = true;
        m_paragraphDir = DirAuto;
        m_bParagraphEmpty = true;
    }else if (tag == "html"){ // we display as a part of a larger document
        return;
    }else if (tag == "head"){
        m_bInHead = 1;
        return;
    }else if (tag == "body"){ // we display as a part of a larger document
        oTag = "span";
    }

    QString tagText;
    tagText += '<';
    tagText += oTag;

    if (tag == "p")
    {
        m_paraDirInsertionPos = res.length() + tagText.length();
    }

    for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
        QString name = it->toLower();
        ++it;
        QString value = *it;

        // Handling for attributes of specific tags.
        if (tag == "body"){
            if (name == "bgcolor"){
                style += "background-color:" + value + ';';
                continue;
            }
        }else if (tag == "p"){
            if (name == "dir"){
                QString dir = value.toLower();
                if (dir == "ltr")
                    m_paragraphDir = DirLTR;
                else if (dir == "rtl")
                    m_paragraphDir = DirRTL;
                else
                    m_paragraphDir = DirUnknown;
            }
        }else if (tag == "font"){
            if (name == "color" && m_bIgnoreColors)
                continue;
        }

        // Handle for generic attributes.
        if (name == "style"){
            style += value;
            continue;
        }

        tagText += ' ';
        tagText += name;
        if (!value.isEmpty()){
            tagText += "=\"";
            tagText += value;
            tagText += '\"';
        }
    }

    // Quite crude but working CSS to remove color styling.
    // It won't filter out colors as part of 'background', but life's tough.
    // (If it's any comfort, Qt probably won't display it either.)
    if (!style.isEmpty()){
        if (m_bIgnoreColors){
            list<QString> opt = parseStyle(style);
            list<QString> new_opt;
            for (list<QString>::iterator it = opt.begin(); it != opt.end(); ++it){
                QString name = *it;
                it++;
                if (it == opt.end())
                    break;
                QString value = *it;
                if ((name == "color") ||
						(name == "background") ||
                        (name == "background-color") ||
                        (name == "font-size") ||
                        (name == "font-style") ||
						(name == "font-variant") ||
                        (name == "font-weight") ||
                        (name == "font-family"))
                    continue;
                new_opt.push_back(name);
                new_opt.push_back(value);
            }
            style = makeStyle(new_opt);
        }
        if (!style.isEmpty())
            tagText += " style=\"" + style + '\"';
    }
    tagText += '>';
    res += tagText;
}

void ViewParser::tag_end(const QString &tag)
{
    QString oTag = tag;
    if (tag == "a"){
        m_bInLink = false;
    }else if (tag == "p"){
        if (m_bInParagraph && m_bParagraphEmpty)
           // The user probably didn't intend to insert an empty paragraph.
           // We are probably viewing faulty content produced by QTextEdit
           // (which returns <p></p> but optimizes it away upon loading),
           // so we fix it up.
           res += "<br>";
        res += "</p>";
        m_bInParagraph = false;
        return;
    }else if (tag == "head"){
        m_bInHead = false;
        return;
    }else if (tag == "html"){
        return;
    }else if (tag == "body"){
        oTag = "span";
    }
    if (m_bInHead)
        return;
    res += "</";
    res += oTag;
    res += '>';
}

QString MsgViewBase::parseText(const QString &text, bool bIgnoreColors, bool bUseSmiles)
{
    ViewParser parser(bIgnoreColors, bUseSmiles);
    return parser.parse(text);
}

/*
#ifndef NO_MOC_INCLUDES
#include "msgview.moc"
#endif
*/

