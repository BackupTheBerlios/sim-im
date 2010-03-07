/***************************************************************************
                          historywnd.h  -  description
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

#ifndef _HISTORYWND_H
#define _HISTORYWND_H

#include <vector>
#include "event.h"

#include <QMainWindow>
#include <QPixmap>
#include <QLabel>
#include <QResizeEvent>

class MsgViewBase;
class CToolBar;
class QToolButton;
class QComboBox;
class HistoryProgressBar;
class HistoryIterator;

class HistoryWindow : public QMainWindow, public SIM::EventReceiver
{
    Q_OBJECT
public:
    HistoryWindow(unsigned long id);
    ~HistoryWindow();
    unsigned long id() { return m_id; }
protected slots:
    void toolbarChanged(QToolBar*);
    void fill();
    void next();
protected:
    virtual bool processEvent(SIM::Event*);
    void resizeEvent(QResizeEvent*);
    void setName();
    void addHistory(const QString &str);
    QStatusBar	*m_status;
    MsgViewBase	*m_view;
    CToolBar	*m_bar;
    CToolBar	*m_avatar_bar;
    QString     m_filter;
    HistoryProgressBar	*m_progress;
    HistoryIterator		*m_it;
    bool	 m_bDirection;
    unsigned m_nMessages;
    unsigned long m_id;
    unsigned m_page;
    std::vector<QString> m_states;
    unsigned m_history_page_count;
};

#endif
