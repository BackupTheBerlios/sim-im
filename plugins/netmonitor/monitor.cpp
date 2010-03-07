/***************************************************************************
                          monitor.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include <qmenubar.h>
#include <QAction>
#include <QMenu>
#include <QFile>
#include <QMessageBox>
#include <QRegExp>
#include <QTimer>
#include <QTextStream>
#include <QCloseEvent>
#include <QFileDialog>
#include <QTextEdit>
#include <QScrollBar>

#include "icons.h"
#include "log.h"
#include "misc.h"
#include "unquot.h"

#include "monitor.h"
#include "netmonitor.h"

using namespace SIM;

MonitorWindow *monitor = NULL;

MonitorWindow::MonitorWindow(NetmonitorPlugin *plugin)
        : QMainWindow(NULL, Qt::Window)
        , m_plugin(plugin)
{
    bPause = true;  // no debug output during creation
    SET_WNDPROC("monitor")
    setWindowTitle(i18n("Network monitor"));
    setWindowIcon(Icon("network"));

    edit = new QTextEdit(this);
    edit->setLineWrapMode(QTextEdit::NoWrap);
    edit->setReadOnly(true);
    setCentralWidget(edit);
    QMenuBar *menu = menuBar();

    QMenu *menuFile = new QMenu(i18n("&File"), menu);
    connect(menuFile, SIGNAL(aboutToShow()), this, SLOT(adjustFile()));
    m_saveAction = menuFile->addAction(Icon("filesave"), i18n("&Save"), this, SLOT(save()));
    menuFile->addSeparator();
    m_autoscrollAction = menuFile->addAction(i18n("&Autoscroll"), this, SLOT(toggleAutoscroll()));
    m_autoscrollAction->setCheckable(true);
    m_pauseAction = menuFile->addAction(i18n("&Pause"), this, SLOT(pause()));
    menuFile->addSeparator();
    menuFile->addAction(Icon("exit"), i18n("E&xit"), this, SLOT(exit()));
    menu->addMenu(menuFile);

    QMenu *menuEdit = new QMenu(i18n("&Edit"), menu);
    connect(menuEdit, SIGNAL(aboutToShow()), this, SLOT(adjustEdit()));
    m_copyAction = menuEdit->addAction(i18n("&Copy"), this, SLOT(copy()));
    m_eraseAction = menuEdit->addAction(i18n("&Erase"), this, SLOT(erase()));
    menu->addMenu(menuEdit);

    m_menuLog = new QMenu(i18n("&Log"), menu);
    connect(m_menuLog, SIGNAL(aboutToShow()), this, SLOT(adjustLog()));
    connect(m_menuLog, SIGNAL(triggered(QAction*)), this, SLOT(toggleType(QAction*)));
    menu->addMenu(m_menuLog);

    bPause = false;
    bAutoscroll = true;
    edit->append( "<pre>" );
}

void MonitorWindow::closeEvent(QCloseEvent *e)
{
    QMainWindow::closeEvent(e);
    emit finished();
}

void MonitorWindow::save()
{
    QString s = QFileDialog::getSaveFileName (this, QString(), QString(), "sim.log");
    if (s.isEmpty())
        return;
    QFile f(s);
    if (!f.open(QIODevice::WriteOnly)){
        QMessageBox::warning(this, i18n("Error"), i18n("Can't create file %1") .arg(s));
        return;
    }
    QTextStream ts(&f);
    QString t;
    if (edit->textCursor().hasSelection()){
        t = unquoteText(edit->textCursor().selectedText());
    }else{
        t = unquoteText(edit->toPlainText());
    }
#if defined(WIN32) || defined(__OS2__)
    t.replace('\n',"\r\n");
#endif
    ts << t;
    f.close();
}

void MonitorWindow::exit()
{
    close();
}

void MonitorWindow::adjustFile()
{
    m_saveAction->setEnabled(edit->textCursor().hasSelection());
    m_pauseAction->setText(bPause ? i18n("&Resume") : i18n("&Pause"));
    m_autoscrollAction->setChecked(bAutoscroll);
}

void MonitorWindow::copy()
{
    edit->copy();
}

void MonitorWindow::erase()
{
    edit->clear();
}

void MonitorWindow::adjustEdit()
{
    m_copyAction->setEnabled(edit->textCursor().hasSelection());
    m_eraseAction->setEnabled(!edit->textCursor().hasSelection());
}

void MonitorWindow::toggleType(QAction *a)
{
    int id = a->data().toInt();
    switch (id){
    case L_DEBUG:
    case L_WARN:
    case L_ERROR:
    case L_PACKETS:
        m_plugin->setValue("LogLevel", m_plugin->value("LogLevel").toUInt() ^ id);
        return;
    }
    m_plugin->setLogType(id, !m_plugin->isLogType(id));
}

void MonitorWindow::toggleAutoscroll()
{
    bAutoscroll = !bAutoscroll;
}

void MonitorWindow::pause()
{
    bPause = !bPause;
}

struct level_def
{
    unsigned	level;
    const char	*name;
};

static level_def levels[] =
    {
        { L_DEBUG, I18N_NOOP("&Debug") },
        { L_WARN, I18N_NOOP("&Warnings") },
        { L_ERROR, I18N_NOOP("&Errors") },
        { L_PACKETS, I18N_NOOP("&Packets") },
        { 0, NULL }
    };

void MonitorWindow::adjustLog()
{
    m_menuLog->clear();
    PacketType *packet;
    ContactList::PacketIterator it;
    while ((packet = ++it) != NULL){
        QAction *a = m_menuLog->addAction(i18n(packet->name()));
        a->setCheckable(true);
        a->setChecked(m_plugin->isLogType(packet->id()));
        a->setData(packet->id());
    }
    m_menuLog->addSeparator();
    for (const level_def *d = levels; d->name; d++){
        QAction *a = m_menuLog->addAction(i18n(d->name));
        a->setCheckable(true);
        a->setChecked((m_plugin->value("LogLevel").toUInt() & d->level) != 0);
        a->setData(d->level);
    }
}

struct LevelColorDef
{
    unsigned	level;
    const char	*color;
};

static LevelColorDef levelColors[] =
    {
        { L_DEBUG,	"008000" },
        { L_WARN,	"808000" },
        { L_ERROR,	"800000" },
        { L_PACKET_IN,	"000080" },
        { L_PACKET_OUT, "000000" },
        { 0,		 NULL 	 }
    };

bool MonitorWindow::processEvent(Event *e)
{
    if (!e) {
        return false;
    }

	EventLog *l = static_cast<EventLog*>(e);

    if (e->type() == eEventLog && !bPause &&
			(
                        ((l->packetID() == 0 && (l->logLevel() & m_plugin->value("LogLevel").toUInt())) ||
                        ( l->packetID()      && ((m_plugin->value("LogLevel").toUInt() & L_PACKETS) || m_plugin->isLogType(l->packetID()))))
			)
		)
	{

		const char *font = NULL;
        for (const LevelColorDef *d = levelColors; d->color; d++)
			if (l->logLevel() == d->level){
				font = d->color;
				break;
			}
		
        QString logString;
		if (font)
			logString += QString("<font color=\"#%1\">") .arg(font);
		QString s = EventLog::make_packet_string(*l);
		logString += quoteString(s);
		if (font)
			logString += QString("</font>");
		QMutexLocker lock(&m_mutex);
		m_logStrings += logString;
		QTimer::singleShot(10, this, SLOT(outputLog()));
    }
    return false;
}

void MonitorWindow::outputLog()
{
    if(m_logStrings.isEmpty())
        return;
    setLogEnable(false);

    QMutexLocker lock(&m_mutex);

    for(int i = 0; i < m_logStrings.count(); i++)
        edit->append(m_logStrings[i]);

    m_logStrings.clear();
    if (bAutoscroll)
    {
        QScrollBar *sb = edit->verticalScrollBar();
        if (NULL != sb)
        {
            sb->setValue(sb->maximum());
        }
    }
    setLogEnable(true);
}

