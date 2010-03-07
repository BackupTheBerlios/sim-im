/***************************************************************************
historywnd.cpp  -  description
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
#include "log.h"

#include "historywnd.h"
#include "core.h"
#include "msgview.h"
#include "history.h"
#include "contacts/contact.h"
#include "contacts/client.h"
#include "simgui/toolbtn.h"

#include <QPixmap>
#include <QTimer>
#include <QComboBox>
#include <QLineEdit>
#include <QStatusBar>
#include <QProgressBar>
#include <QLayout>
#include <QStringList>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QLabel>

#ifdef USE_KDE
#include <kfiledialog.h>
#define QFileDialog KFileDialog
#else
#include <QFileDialog>
#endif

#include <time.h>

using namespace SIM;

class HistoryProgressBar : public QWidget
{
public:
    HistoryProgressBar(QWidget *parent);
    void setTotalSteps(unsigned);
    void setProgress(unsigned);
protected:
    QProgressBar	*m_bar;
    QHBoxLayout		*m_lay;
    QLabel			*m_label;
};

HistoryProgressBar::HistoryProgressBar(QWidget *parent)
: QWidget(parent)
, m_bar		(new QProgressBar(this))
, m_lay     (new QHBoxLayout(this))
, m_label	(new QLabel(i18n("Loading"), this))
{
    m_lay->setSpacing(4);
    m_lay->addSpacing(4);
    m_lay->addWidget(m_label);
    m_lay->addWidget(m_bar);
}

void HistoryProgressBar::setTotalSteps(unsigned n)
{
    m_bar->setMaximum(n);
}

void HistoryProgressBar::setProgress(unsigned n)
{
    m_bar->setValue(n);
}

HistoryWindow::HistoryWindow(unsigned long id)
: m_status(statusBar())
, m_view(new MsgViewBase(this, NULL, id))
, m_avatar_bar(NULL)
, m_progress(NULL)
, m_it(NULL)
, m_id(id)
, m_page(0)

{
    m_history_page_count=CorePlugin::instance()->value("HistoryPage").toUInt();

    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowIcon(Icon("history"));
    setName();
    setCentralWidget(m_view);

    setIconSize(QSize(16,16));

    EventToolbar eHistoryBar(ToolBarHistory, this);
    eHistoryBar.process();

    m_bar=eHistoryBar.toolBar();
    m_bar->setParam((void*)m_id); //UAAARGH turns my stomach, Fixme

    //restoreToolbar(m_bar, CorePlugin::instance()->data.HistoryBar);
    connect(m_bar, SIGNAL(movableChanged(bool)), this, SLOT(toolbarChanged(bool)));
    addToolBar(m_bar);

    Command cmdHistory;
    cmdHistory->id	= CmdHistoryFind;
    cmdHistory->param	= (void*)m_id; //Fixme ...
    EventCommandWidget eHistoryWidget(cmdHistory);
    eHistoryWidget.process();

    CToolCombo *cmbFind = qobject_cast<CToolCombo*>(eHistoryWidget.widget());
    if(cmbFind)
    {
        const QStringList history = CorePlugin::instance()->value("HistorySearch").toString().split(';');
        cmbFind->addItems(history);
        cmbFind->setText(QString());
    }

    m_bDirection = CorePlugin::instance()->value("HistoryDirection").toBool();
    m_bar->checkState();
    m_bar->show();

    if(CorePlugin::instance()->value("ShowAvatarInHistory").toBool())
    {
        unsigned j = 0;
        QImage img;
        while(j < getContacts()->nClients())
        {
            Client *client = getContacts()->getClient(j++);
            img = client->userPicture(id);
            if (!img.isNull())
                break;
        }

        if(!img.isNull())
        {
            EventToolbar(ToolBarHistoryAvatar, EventToolbar::eAdd).process();
            EventToolbar e(ToolBarHistoryAvatar, this);
            e.process();
            m_avatar_bar = e.toolBar();
            m_avatar_bar->setOrientation(Qt::Vertical);
            addToolBar(m_avatar_bar);

            //m_avatar_bar->setHorizontalStretchable(false);
            //m_avatar_bar->setVerticalStretchable(false);
            m_avatar_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            //restoreToolbar(m_avatar_bar, CorePlugin::instance()->data.HistoryAvatarBar);

            Command cmd;
            cmd->id = CmdHistoryAvatar;
            cmd->bar_id = ToolBarHistoryAvatar;
            cmd->bar_grp	 = 0x2000;
            cmd->text = QString::null;
            cmd->icon = "empty";
            cmd->flags = BTN_LABEL;

            EventCommandCreate(cmd).process();

            Command cmdw;
            cmdw->id	= CmdHistoryAvatar;
            EventCommandWidget eWidget(cmdw);
            eWidget.process();
            CToolLabel *lblAvatar = qobject_cast<CToolLabel*>(eWidget.widget());

            if(lblAvatar)
            {
                lblAvatar->setPixmap(QPixmap::fromImage(img));
            }
            m_avatar_bar->checkState();
            m_avatar_bar->show();
            //m_avatar_bar->area()->moveDockWindow(m_avatar_bar, 0);
        }
    }

    fill();
}

HistoryWindow::~HistoryWindow()
{
    delete m_avatar_bar;
    delete m_it;
    delete m_progress; //??
    delete m_bar;  //??
    delete m_view; //??
}

void HistoryWindow::setName()
{
    QString name;
    Contact *contact = getContacts()->contact(m_id);
    if (contact)
        name = contact->getName();
    setWindowTitle(i18n("History") + ' ' + name);
}

bool HistoryWindow::processEvent(Event *e)
{
    switch(e->type()) {
case eEventContact: 
    {
        EventContact *ec = static_cast<EventContact*>(e);
        Contact *contact = ec->contact();
        if (contact->id() != m_id)
            break;
        switch(ec->action()) 
        {
        case EventContact::eDeleted:
            QTimer::singleShot(0, this, SLOT(close()));
            break;
        case EventContact::eChanged:
            setName();
            break;
        default:
            break;
        }
        break;
    }
case eEventCheckCommandState: 
    {
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if (cmd->id == CmdHistoryDirection && (unsigned long)(cmd->param) == m_id)
        {
            cmd->flags &= ~COMMAND_CHECKED;
            if (m_bDirection)
                cmd->flags |= COMMAND_CHECKED;
            return true;
        }
        if (cmd->id != CmdDeleteMessage && cmd->id != CmdCutHistory || cmd->param != m_view || !m_view->currentMessage())
            return false;

        cmd->flags &= ~COMMAND_CHECKED;
        return true;
    }
case eEventCommandExec: 
    {
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if ((unsigned long)(cmd->param) != m_id)
            return false;
        if (cmd->id == CmdHistoryDirection)
        {
            bool bDirection = ((cmd->flags & COMMAND_CHECKED) != 0);
            CorePlugin::instance()->setValue("HistoryDirection", bDirection);
            if (bDirection != m_bDirection)
            {
                m_bDirection = bDirection;
                m_page = 0;
                m_states.clear();
                fill();
            }
            return true;
        }
        if (cmd->id == CmdHistoryNext)
        {
            if (m_page + 1 < m_states.size())
            {
                m_page++;
                fill();
            }
            return true;
        }
        if (cmd->id == CmdHistoryPrev)
        {
            if (m_page > 0)
            {
                m_page--;
                fill();
            }
            return true;
        }
        if (cmd->id == CmdHistorySave)
        {
            QString str = QFileDialog::getSaveFileName(this, QString::null, QString::null, i18n("Textfile (*.txt)"));
            if(!str.isEmpty())
            {
                bool res = true;
                if (QFile::exists(str))
                {
                    QMessageBox mb(i18n("Error"), i18n("File already exists. Overwrite?"), 
                        QMessageBox::Warning,
                        QMessageBox::Yes | QMessageBox::Default,
                        QMessageBox::No,
                        QMessageBox::Cancel | QMessageBox::Escape);
                    mb.setButtonText(QMessageBox::Yes, i18n("&Overwrite"));
                    mb.setButtonText(QMessageBox::No, i18n("&Append"));
                    switch (mb.exec())
                    {
                    case QMessageBox::Yes:
                        res = History::save(m_id, str, false);
                        break;
                    case QMessageBox::No:
                        res = History::save(m_id, str, true);
                        break;
                    case QMessageBox::Cancel:
                        break;
                    }
                }else
                    res = History::save(m_id, str);
                if (!res)
                    QMessageBox::critical(this, i18n("Error"), i18n("Save failed"), QMessageBox::Ok, Qt::NoButton, Qt::NoButton);
            }
            return true;
        }
        if (cmd->id == CmdHistoryFind)
        {
            m_filter.clear();
            if (cmd->flags & COMMAND_CHECKED)
            {
                Command cmd;
                cmd->id		= CmdHistoryFind;
                cmd->param	= (void*)m_id;
                EventCommandWidget eWidget(cmd);
                eWidget.process();
                CToolCombo *cmbFind = qobject_cast<CToolCombo*>(eWidget.widget());
                QString text = cmbFind->lineEdit()->text();
                if (cmbFind && !text.isEmpty())
                {
                    addHistory(text);
                    m_filter = text;
                }
            }
            m_page = 0;
            m_states.clear();
            m_view->setSelect(m_filter);
            fill();
            return true;
        }
        break;
    }
    default:
        break;
    }
    return false;
}

void HistoryWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    //CorePlugin::instance()->data.HistorySize[0].asULong() = width();
    //CorePlugin::instance()->data.HistorySize[1].asULong() = height();
}

void HistoryWindow::toolbarChanged(QToolBar*)
{
    //saveToolbar(m_bar, CorePlugin::instance()->data.HistoryBar);
    //saveToolbar(m_avatar_bar, CorePlugin::instance()->data.HistoryAvatarBar);
}

void HistoryWindow::fill()
{
    log(L_DEBUG, "Fill");
    if (m_it == NULL)
        m_it = new HistoryIterator(m_id);
    if (m_progress == NULL)
    {
        m_progress = new HistoryProgressBar(m_status);
        m_status->addWidget(m_progress, 1);
    }
    m_it->setFilter(m_filter);
    m_progress->setTotalSteps(m_history_page_count);
    m_progress->setProgress(0);
    m_progress->show();
    m_nMessages = 0;
    if (m_bDirection)
        m_it->end();
    else
        m_it->begin();
    if (m_states.size())
        m_it->setState(m_states[m_page]);
    else
        m_states.push_back(m_it->state());
    m_view->setText(QString::null);
    QTimer::singleShot(0, this, SLOT(next()));
    Command cmd;
    cmd->id     = CmdHistoryNext;
    cmd->flags	= COMMAND_DISABLED;
    cmd->param	= (void*)m_id;
    EventCommandDisabled(cmd).process();
    cmd->id     = CmdHistoryPrev;
    cmd->flags  = (m_page > 0) ? 0 : COMMAND_DISABLED;
    EventCommandDisabled(cmd).process();
}

void HistoryWindow::next()
{
    if ( m_it == NULL )
        return;

    //Quickfix Noragen, Stop at 1000 Messages, if there are Problems with storing the size.
    if (m_history_page_count > 1000)
        m_history_page_count=1000;
		
    m_progress->setTotalSteps(m_history_page_count);

    for (;;)
    {
        QString state = m_it->state();
        Message *msg = NULL;
        if (m_bDirection)
            msg = --(*m_it);
        else
            msg = ++(*m_it);

        if (++m_nMessages > m_history_page_count && msg)
        {
            Command cmd;
            cmd->id		= CmdHistoryNext;
            cmd->flags  = 0;
            cmd->param	= (void*)m_id;
            EventCommandDisabled(cmd).process();
            msg = NULL;
            if (m_page+1>=m_states.size())
                m_states.push_back(state);
        }

        if (msg == NULL)
            break;

        m_view->addMessage(msg, false, false);
        m_progress->setProgress(m_nMessages);
    }

    delete m_progress;
    delete m_it;
    m_it = NULL;
    m_progress = NULL;
    log(L_DEBUG, "Stop");
}

const unsigned MAX_HISTORY = 10;

void HistoryWindow::addHistory(const QString &str)
{
    QStringList l = CorePlugin::instance()->value("HistorySearch").toString().split(';');
    l.removeAll(str);
    l.prepend(str);

    QString res;
    unsigned i = 0;
    Q_FOREACH(const QString &str, l)
    {
        if (i++ > MAX_HISTORY)
            break;
        if (!res.isEmpty())
            res += ';';
        res += quoteChars(str, ";");
    }
    CorePlugin::instance()->setValue("HistorySearch", res);
}

