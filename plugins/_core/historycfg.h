/***************************************************************************
                          historycfg.h  -  description
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

#ifndef _HISTORYCFG_H
#define _HISTORYCFG_H

#include <vector>

#include "simgui/qcolorbutton.h"
#include <QEvent>
#include "ui_historycfgbase.h"

class CorePlugin;
class QSyntaxHighlighter;

struct StyleDef
{
    QString name;
    QString text;
    bool	bCustom;
    bool	bChanged;
    bool	operator < (const StyleDef &s) const { return name < s.name; }
};

class HistoryConfig : public QWidget, public Ui::HistoryConfigBase
{
    Q_OBJECT
public:
    HistoryConfig(QWidget *parent);
    ~HistoryConfig();
public slots:
    void apply();
    void styleSelected(int);
    void copy();
    void rename();
    void del();
    void realDelete();
    void realRename();
    void cancelRename();
    void viewChanged(QWidget*);
    void textChanged();
    void toggled(bool);
    void toggledDays(bool);
    void toggledSize(bool);
    void toggledExtViewer(bool);
protected:
    void fillPreview();
    bool eventFilter(QObject *o, QEvent *e);
    void addStyles(const QString &dir, bool bName);
    void fillCombo(const QString &current);
    bool m_bDirty;
    int  m_cur;
    int  m_edit;
    std::vector<StyleDef> m_styles;
    QSyntaxHighlighter* highlighter;
};

#endif

