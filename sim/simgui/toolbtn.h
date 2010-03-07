/***************************************************************************
                          toolbtn.h  -  description
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

#ifndef _TOOLBTN_H
#define _TOOLBTN_H

#include "simapi.h"
#include "event.h"

#include <QToolBar>
#include <QToolButton>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>

class QContextMenuEvent;
class QHideEvent;
class QMainWindow;
class QMouseEvent;
class QShowEvent;
class CToolItem;
typedef QHash<unsigned, CToolItem*> ButtonsMap;

// Base class for all Widgets in CToolBar
class EXPORT CToolItem
{
public:
    CToolItem(SIM::CommandDef *def);
    virtual ~CToolItem() {}
    virtual void setState();
    virtual QWidget *widget() = 0;
    virtual CToolItem *subItem() = 0;
    QAction *action() { return m_action; }
    void setAction(QAction *a) { m_action = a; }
    void checkState();
    void setCommand(SIM::CommandDef *def);
    void setChecked(SIM::CommandDef *def);
    void setDisabled(SIM::CommandDef *def);
    void setShow(SIM::CommandDef *def);
    const SIM::CommandDef &def() const { return m_def; }
    void setDef(const SIM::CommandDef &def) { m_def = def; }
protected:
    SIM::CommandDef m_def;
    QString m_text;
    QAction *m_action;
};

// A simple QToolButton -> type: BTN_DEFAULT
class EXPORT CToolButton : public QToolButton, public CToolItem
{
    Q_OBJECT
public:
    CToolButton(CToolBar *parent, SIM::CommandDef *def);
    ~CToolButton();
    virtual void setState();
    virtual QWidget *widget() { return this; }
    virtual CToolItem *subItem() { return NULL; }
    static QPoint popupPos(QWidget*, QWidget*);
    void setTextLabel(const QString &text);
signals:
    void showPopup(QPoint);
    void buttonDestroyed();
protected slots:
    void btnClicked();
    void btnToggled(bool);
    void accelActivated(int);
protected:
    void setTextLabel();
    void mousePressEvent(QMouseEvent*);
    void contextMenuEvent(QContextMenuEvent*);
    void showEvent(QShowEvent*);
    void hideEvent(QHideEvent*);
    void setAccel(int key);
    void enableAccel(bool bState);
    unsigned accelKey;
    QPoint popupPos(QWidget *p);
};

// A simple QToolButton with an picture -> type: BTN_PICT
class EXPORT CToolPictButton : public CToolButton
{
    Q_OBJECT
public:
    CToolPictButton(CToolBar *parent, SIM::CommandDef *def);
    ~CToolPictButton();
protected:
    virtual void setState();
    void paintEvent(QPaintEvent*);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
};

// A QComboBox -> type: BTN_COMBO or BTN_COMBO_CHECK
class EXPORT CToolCombo : public QComboBox, public CToolItem
{
    Q_OBJECT
public:
    CToolCombo(CToolBar *parent, SIM::CommandDef *def, bool bCheck);
    ~CToolCombo();
    virtual QWidget *widget() { return this; }
    virtual CToolItem *subItem() { return m_btn; }
    void setText(const QString&);
protected slots:
    void slotTextChanged(const QString &str);
    void btnDestroyed();
protected:
    virtual QSizePolicy sizePolicy() const;
    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;
    virtual void setState();
    CToolButton	*m_btn;
    bool m_bCheck;
};

// A  QLineEdit -> type: BTN_EDIT
class EXPORT CToolEdit : public QLineEdit, public CToolItem
{
    Q_OBJECT
public:
    CToolEdit(CToolBar *parent, SIM::CommandDef *def);
    ~CToolEdit();
    virtual QWidget *widget() { return this; }
    virtual CToolItem *subItem() { return m_btn; }
protected slots:
    void btnDestroyed();
    virtual QSizePolicy sizePolicy() const;
    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;
protected:
    virtual void setState();
    CToolButton	*m_btn;
};

// A simple QLabel -> type: BTN_LABEL
class EXPORT CToolLabel : public QLabel, public CToolItem
{
    Q_OBJECT
public:
    CToolLabel(CToolBar *parent, SIM::CommandDef *def);
    ~CToolLabel();
    virtual QWidget *widget() { return this; }
    virtual CToolItem *subItem() { return NULL; }
protected:
    virtual QSizePolicy sizePolicy() const;
    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;
};


class EXPORT CToolBar : public QToolBar, public SIM::EventReceiver
{
    Q_OBJECT
public:
    CToolBar(SIM::CommandsDef *def, QMainWindow *parent);
    ~CToolBar();
    SIM::CommandsDef *m_def;
    void *param()				{ return m_param; }
    void setParam(void *param)	{ m_param = param; }
    virtual bool processEvent(SIM::Event *e);
public slots:
    void showPopup(QPoint p);
    void toolBarChanged();
    void checkState();
protected:
    void	mousePressEvent(QMouseEvent *e);
    void	contextMenuEvent(QContextMenuEvent *e);
    bool	bChanged;
    void	*m_param;
    ButtonsMap  buttons;
    virtual QSizePolicy sizePolicy() const;
};

#endif

