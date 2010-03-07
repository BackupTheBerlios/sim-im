/***************************************************************************
                          cmenu.cpp  -  description
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

#include "cmddef.h"
#include "misc.h"
#include "icons.h"
#include "cmenu.h"
#include "commands.h"

#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <QMenuItem>

using namespace SIM;

CMenu::CMenu(CommandsDef *def)
    : QMenu(NULL)
    , m_def(def)
    , m_param(NULL)
    , m_bInit(false)
{
    connect(this, SIGNAL(aboutToShow()), this, SLOT(showMenu()));
    connect(this, SIGNAL(aboutToHide()), this, SLOT(hideMenu()));
    connect(this, SIGNAL(triggered(QAction*)), this, SLOT(menuActivated(QAction*)));
}

CMenu::~CMenu()
{
}

void CMenu::setParam(void *param)
{
    m_param = param;
}

void CMenu::processItem(CommandDef *s, bool &bSeparator, bool &bFirst, unsigned base_id)
{
    if (s->id == 0)
    {
        bSeparator = true;
        return;
    }
    s->param = m_param;
    if (s->flags & COMMAND_CHECK_STATE)
    {
        s->flags &= ~COMMAND_DISABLED;
        s->text_wrk = QString::null;
        s->flags |= COMMAND_CHECK_STATE;  // FIXME: What for? COMMAND_CHECK_STATE BIT is already 1 if this code is execued, because of "if" above
        if(!EventCheckCommandState(s).process())
            return;
        if (s->flags & COMMAND_RECURSIVE)
        {
            CommandDef *cmds = (CommandDef*)(s->param);
            for (CommandDef *cmd = cmds; !cmd->text.isEmpty(); cmd++)
            {
                processItem(cmd, bSeparator, bFirst, s->id);
            }
            delete[] cmds;
            s->param = NULL;
            return;
        }
    }
    if(s->flags & BTN_HIDE)
        return;
    if (m_wrk->actions().count())
    {
        QSize s = m_wrk->sizeHint();
        QWidget *desktop = qApp->desktop();
        int nHeight = (s.height() - 2 * 2) / m_wrk->actions().count();
        if (s.height() + nHeight * 2 + 2 * 2 >= desktop->height()){
            QAction *pAction = m_wrk->addAction(i18n("More..."));
            QMenu *more = new QMenu(m_wrk);
            pAction->setMenu(more);
            m_wrk = more;
            connect(m_wrk, SIGNAL(triggered(QAction*)), this, SLOT(menuActivated(QAction*)));
        }
    }
    if (bFirst)
    {
        bFirst = false;
        bSeparator = false;
    }
    else if (bSeparator)
    {
        m_wrk->addSeparator();
        bSeparator = false;
    }
    QIcon icons;
    if ((s->flags & COMMAND_CHECKED) && !s->icon_on.isEmpty())
        icons = Icon(s->icon_on);
    if (icons.isNull() && !s->icon.isEmpty())
        icons = Icon(s->icon);
    QString title = i18n(s->text);
    if (!s->text_wrk.isEmpty())
    {
        title = s->text_wrk;
        s->text_wrk = QString::null;
    }
    if (!s->accel.isEmpty())
    {
        title += '\t';
        title += i18n(s->accel);
    }
    if (s->flags & COMMAND_TITLE)
    {
        if (!icons.isNull())
        {
//          m_wrk->insertTitle(icons.pixmap(QIcon::Automatic, QIcon::Normal), title);
        }
        else
        {
//          m_wrk->insertTitle(title);
        }
        bFirst = true;
        bSeparator = false;
        return;
    }
    QMenu *popup = NULL;
    if (s->popup_id)
    {
        EventMenuProcess e(s->popup_id, s->param, 0);
        e.process();
        popup = e.menu();
    }
    unsigned id = 0;
    QAction *pAction = m_wrk->addAction(icons, title);
    if (popup)
    {
        pAction->setMenu(popup);
    }
    else
    {
        CMD c;
        c.id = s->id;
        c.base_id = base_id;
        m_cmds.push_back(c);
        id = m_cmds.size();
        pAction->setData(QVariant(id));
    }
    if (id)
    {
        if (s->flags & COMMAND_DISABLED)
            pAction->setEnabled(false);
        if (!s->accel.isEmpty())
            pAction->setShortcut(QKeySequence::fromString(i18n(s->accel)));
        if ((s->flags & COMMAND_CHECKED) != 0)
        {
            pAction->setCheckable(true);
            pAction->setChecked((s->flags & COMMAND_CHECKED) != 0);
        }
    }
    bSeparator = false;
}

void CMenu::showMenu()
{
    initMenu();
}

void CMenu::hideMenu()
{
    m_bInit = false;
}

void CMenu::clearMenu()
{
    clear();
}

QSize CMenu::sizeHint() const
{
    ((CMenu*)this)->initMenu();
    return QMenu::sizeHint();
}

void CMenu::initMenu()
{
    if (m_bInit)
        return;
    m_bInit = true;
    m_wrk   = this;
    clear();
    m_cmds.clear();
    bool bSeparator = false;
    bool bFirst = true;
    CommandsList list(*m_def);
    CommandDef *s;
    while ((s = ++list) != NULL)
	{
        processItem(s, bSeparator, bFirst, 0);
    }
}

void CMenu::menuActivated(QAction *pAction)
{
    int n = pAction->data().toInt();

    if ((n < 1) || (n > (int)(m_cmds.size())))
        return;

    CMD c = m_cmds[n - 1];
    unsigned id = c.id;
    if (c.base_id)
        id = c.base_id;

    CommandsList list(*m_def, true);
    CommandDef *s;
    while ((s = ++list) != NULL){
        if (s->id == id){
            s->text_wrk = QString::null;
            if (s->flags & COMMAND_CHECK_STATE){
                s->param = m_param;
                s->flags |= COMMAND_CHECK_STATE;
                if(!EventCheckCommandState(s).process()){
                    s->text_wrk = QString::null;
                    return;
                }
                s->flags ^= COMMAND_CHECKED;
                if (s->flags & COMMAND_RECURSIVE){
                    CommandDef *cmds = (CommandDef*)(s->param);
                    delete[] cmds;
                }
            }
            if (c.base_id)
                s->id = c.id;
            s->param = m_param;
            EventCommandExec(s).process();
            s->text_wrk = QString::null;
            s->id = id;
            break;
        }
    }
}

