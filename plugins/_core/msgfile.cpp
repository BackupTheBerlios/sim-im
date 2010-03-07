/***************************************************************************
                          msgfile.cpp  -  description
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

#include "msgfile.h"
#include "msgedit.h"
#include "userwnd.h"
#include "core.h"

#include "simgui/toolbtn.h"
#include "simgui/textshow.h"


#include <QTimer>
#include <QToolButton>
#include <QRegExp>

#ifdef USE_KDE
#include <kfiledialog.h>
#define QFileDialog	KFileDialog
#else
#include <QFileDialog>
#endif

using namespace SIM;

MsgFile::MsgFile(MsgEdit *parent, Message *msg)
        : QObject(parent)
{
    m_client   = msg->client();
    m_edit     = parent;
    m_bCanSend = false;
    if (m_edit->m_edit->isReadOnly()){
        m_edit->m_edit->setText(QString::null);
        m_edit->m_edit->setReadOnly(false);
    }
    QString t = msg->getPlainText();
    if (!t.isEmpty())
        m_edit->m_edit->setText(t);

    Command cmd;
    cmd->id		= CmdFileName;
    cmd->param	= parent;
    EventCommandWidget eWidget(cmd);
    eWidget.process();
    CToolEdit *edtName = qobject_cast<CToolEdit*>(eWidget.widget());
    if (edtName){
        connect(edtName, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
        edtName->setText(static_cast<FileMessage*>(msg)->getFile());
    }
    changed(static_cast<FileMessage*>(msg)->getFile());
}

void MsgFile::init()
{
    if (!m_edit->topLevelWidget()->isActiveWindow() || m_edit->topLevelWidget()->isMinimized())
        return;
    Command cmd;
    cmd->id		= CmdFileName;
    cmd->param	= m_edit;
    EventCommandWidget eWidget(cmd);
    eWidget.process();
    CToolEdit *edtName = qobject_cast<CToolEdit*>(eWidget.widget());
    if (edtName){
        if (edtName->text().isEmpty()){
            selectFile();
            return;
        }
        edtName->setFocus();
    }
}

void MsgFile::changed(const QString &str)
{
    if (m_bCanSend != str.isEmpty())
        return;
    m_bCanSend = !str.isEmpty();
    Command cmd;
    cmd->id    = CmdSend;
    cmd->flags = m_bCanSend ? 0 : COMMAND_DISABLED;
    cmd->param = m_edit;
    EventCommandDisabled(cmd).process();
}

void MsgFile::selectFile()
{
    Command cmd;
    cmd->id		= CmdFileName;
    cmd->param	= m_edit;
    EventCommandWidget eWidget(cmd);
    eWidget.process();
    CToolEdit *edtName = qobject_cast<CToolEdit*>(eWidget.widget());
    if (edtName == NULL)
        return;
    QString s = edtName->text();
    QStringList lst = QFileDialog::getOpenFileNames(m_edit->topLevelWidget());
    if ((lst.count() > 1) || ((lst.count() > 0) && (lst[0].indexOf(' ') >= 0))){
        for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it){
            *it = '\"' + QDir::convertSeparators(*it) + '\"';
        }
    }else{
        for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it){
            *it = QDir::convertSeparators(*it);
        }
    }
    edtName->setText(lst.join(" "));
}

bool MsgFile::processEvent(Event *e)
{
    if (e->type() == eEventCheckCommandState){
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if (cmd->param == m_edit){
            unsigned id = cmd->bar_grp;
            if ((id >= MIN_INPUT_BAR_ID) && (id < MAX_INPUT_BAR_ID)){
                cmd->flags |= BTN_HIDE;
                if (cmd->id == CmdFileName)
                    cmd->flags &= ~BTN_HIDE;
                return true;
            }
            switch (cmd->id){
            case CmdTranslit:
            case CmdSmile:
            case CmdSend:
            case CmdSendClose:
                e->process(this);
                cmd->flags &= ~BTN_HIDE;
                return true;
            case CmdNextMessage:
            case CmdMsgAnswer:
                e->process(this);
                cmd->flags |= BTN_HIDE;
                return true;
            }
        }
    } else
    if (e->type() == eEventCommandExec){
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if (cmd->param == m_edit){
            if (cmd->id == CmdSend){
                Command cmd;
                cmd->id		= CmdFileName;
                cmd->param	= m_edit;
                EventCommandWidget eWidget(cmd);
                eWidget.process();
                CToolEdit *edtName = qobject_cast<CToolEdit*>(eWidget.widget());
                if (edtName == NULL)
                    return false;
                QString msgText = m_edit->m_edit->toPlainText();
                QString file = edtName->text();
                QStringList files;
                QString f;
                for (int i = 0; i < (int)file.length(); i++){
                    if (file[i] == '\"'){
                        f = f.trimmed();
                        if (!f.isEmpty())
                            files.append(f);
                        f = QString::null;
                        for (i++; i < (int)file.length(); i++){
                            if (file[i] == '\"')
                                break;
                            f += file[i];
                        }
                        f = f.trimmed();
                        if (!f.isEmpty())
                            files.append(f);
                        f = QString::null;
                        continue;
                    }
                    f += file[i];
                }
                f = f.trimmed();
                if (!f.isEmpty())
                    files.append(f);
                file = QString::null;
                for (QStringList::Iterator it = files.begin(); it != files.end(); ++it){
                    if (!file.isEmpty())
                        file += ';';
                    file += quoteChars(*it, ";");
                }
                if (!file.isEmpty()){
                    FileMessage *msg = new FileMessage;
                    msg->setText(msgText);
                    msg->setFile(file);
                    msg->setContact(m_edit->m_userWnd->id());
                    msg->setClient(m_client);
                    m_edit->sendMessage(msg);
                }
                return true;
            }
            if (cmd->id == CmdFileName){
                selectFile();
                return true;
            }
        }
    }
    return false;
}
