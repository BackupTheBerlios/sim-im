/***************************************************************************
                          editfile.h  -  description
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

#ifndef _EDITFILE_H
#define _EDITFILE_H

#include "simapi.h"

#include <QFrame>
#include <QLineEdit>
#include <QTextEdit>
#include <QItemDelegate>

class QHBoxLayout;
class FilePreview;
class QMenu;

typedef FilePreview *CreatePreview(QWidget *parent);

class EXPORT EditFile : public QFrame
{
    Q_OBJECT
public:
    EditFile(QWidget *p);
    ~EditFile();
    void setText(const QString&);
    QString text() const;
    void setFilter(const QString &filter);
    void setDirMode(bool bMode) { bDirMode = bMode; }
    void setStartDir(const QString &dir);
    void setMultiplyMode(bool bMode) { bMultiplyMode = bMode; }
    void setFilePreview(CreatePreview*);
    void setTitle(const QString &title);
    void setReadOnly(bool);
    void setCreate(bool);
    void setShowHidden(bool);
signals:
    void textChanged(const QString&);
public slots:
    void showFiles();
    void editTextChanged(const QString&);
protected:
    bool bIsImage;
    bool bDirMode;
    bool bMultiplyMode;
    bool bCreate;
    bool bShowHidden;
    QString filter;
    QString startDir;
    QString title;
    QHBoxLayout   *lay;
    QLineEdit     *edtFile;
    CreatePreview *createPreview;
};

class EXPORT EditSound : public EditFile
{
    Q_OBJECT
public:
    EditSound(QWidget *p);
    ~EditSound();
protected slots:
    void play();
};

class EXPORT EditSoundDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    EditSoundDelegate(int column, QObject *parent = 0);

    // QItemDelegate overrides
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const;
protected:
    int m_column;
};

class EXPORT LineEdit : public QLineEdit
{
    Q_OBJECT
public:
    LineEdit(QWidget *parent = 0);
    void setHelpList(const QHash<QString, QByteArray> &helpList);
    void setHelpList(const char **helpList);

protected Q_SLOTS:
    void menuTriggered(QAction*);
protected:
    void contextMenuEvent(QContextMenuEvent *e);
    QHash<QString, QByteArray> m_helpList;
};

class EXPORT LineEditDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    LineEditDelegate(int column, QObject *parent = 0);
    void setHelpList(const QHash<QString, QByteArray> &helpList);
    void setHelpList(const char **helpList);

    // QItemDelegate overrides
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const;
protected:
    int m_column;
    QHash<QString, QByteArray> m_helpList;
};

class EXPORT MultiLineEdit : public QTextEdit
{
    Q_OBJECT
public:
    MultiLineEdit(QWidget *parent);
    void setHelpList(const QHash<QString, QByteArray> &helpList);
    void setHelpList(const char **helpList);

protected Q_SLOTS:
    void menuTriggered(QAction*);
protected:
    void contextMenuEvent(QContextMenuEvent *e);
    QHash<QString, QByteArray> m_helpList;
};

#endif

