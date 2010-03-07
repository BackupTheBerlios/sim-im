/***************************************************************************
                          textshow.h  -  description
                             -------------------
    begin                : Sat Mar 16 2002
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

#ifndef _TEXTSHOW_H
#define _TEXTSHOW_H

#include "simapi.h"
#include "event.h"
#include "core_consts.h"
#include "msggen.h"

#include <qglobal.h>
#include <QMainWindow>
#include <QToolButton>
#include <QLabel>

#include <QTextBrowser>
#include <QFrame>
#include <QGridLayout>

#ifdef USE_KDE
#include <ktextedit.h>
    #define QTextEdit KTextEdit
#endif

class CToolBar;
class QPopupMenu;

const unsigned TextCmdBase	= 0x00030000;
const unsigned CmdBgColor	= TextCmdBase;
const unsigned CmdFgColor	= TextCmdBase + 1;
const unsigned CmdBold		= TextCmdBase + 2;
const unsigned CmdItalic	= TextCmdBase + 3;
const unsigned CmdUnderline	= TextCmdBase + 4;
const unsigned CmdFont		= TextCmdBase + 5;

class EXPORT TextShow : public QTextBrowser
{
    Q_OBJECT
public:
    TextShow (QWidget *parent, const char *name=NULL);
    ~TextShow();
public slots:
    virtual void setSource(const QString &url);
    virtual void setSource(const QUrl &url);
    const QColor &background() const;
    const QColor &foreground() const;
    void setForeground(const QColor&);
    void setBackground(const QColor&);
    void setURL(const QUrl & src);
signals:
    void finished();
protected:
    void emitLinkClicked(const QString&);
    bool linksEnabled() const { return true; }
};

class EXPORT TextEdit : public TextShow, public SIM::EventReceiver
{
    Q_OBJECT
public:
    TextEdit(QWidget *parent, const char *name = NULL);
    ~TextEdit();
    void setCtrlMode(bool);
    const QColor &foreground() const;
    const QColor &defForeground() const;
    void setForeground(const QColor&, bool bDef);
    void setParam(void*);
    void setFont(const QFont&);
    bool isEmpty();
    QPoint m_popupPos;
signals:
    void ctrlEnterPressed();
    void lostFocus();
    void emptyChanged(bool);
    void colorsChanged();
    void fontSelected(const QFont &font);
    void finished(TextEdit*);
protected slots:
    void slotClicked(int parag, int index);
    void slotTextChanged();
    void slotColorChanged(const QColor &c);
    void bgColorChanged(QColor c);
    void fgColorChanged(QColor c);
    void fontChanged(const QFont &f);
protected:
    virtual bool processEvent(SIM::Event *e);
    virtual void focusOutEvent(QFocusEvent *e);
    void keyPressEvent(QKeyEvent *e);
    QMenu *createPopupMenu(const QPoint& pos);
    void *m_param;
    bool m_bBold;
    bool m_bItalic;
    bool m_bUnderline;
    bool m_bChanged;
    QColor curFG;
    QColor defFG;
    bool m_bCtrlMode;
    bool m_bEmpty;
    bool m_bSelected;
    bool m_bNoSelected;
    bool m_bInClick;
};

class EXPORT ColorLabel : public QLabel
{
    Q_OBJECT
public:
    ColorLabel(QWidget *parent, const QColor &c, int id, const QString&);
Q_SIGNALS:
    void selected(int);
protected:
    void mouseReleaseEvent(QMouseEvent*);
    QSize minimumSizeHint() const;
    unsigned m_id;
};

class EXPORT ColorPopup : public QFrame
{
    Q_OBJECT
public:
    ColorPopup(QWidget *parent, const QColor &c);
Q_SIGNALS:
    void colorChanged(QColor color);
protected Q_SLOTS:
    void colorSelected(int);
protected:
    QColor m_color;
	QGridLayout *lay;
};

#endif
