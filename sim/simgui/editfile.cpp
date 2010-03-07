/***************************************************************************
                          editfile.cpp  -  description
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <QLineEdit>
#include <QPushButton>
#include <QIcon>
#include <QLayout>
#include <QStringList>
#include <QApplication>
#include <QRegExp>
#include <QMenu>
#include <QClipboard>
#include <QHBoxLayout>
#include <QFrame>
#include <QFileDialog>
#include <QContextMenuEvent>

#include "editfile.h"
#include "event.h"
#include "icons.h"
#include "misc.h"
#include "preview.h"
#include "unquot.h"

using namespace SIM;

EditFile::EditFile(QWidget *p)
  : QFrame(p)
{
    bDirMode = false;
    bMultiplyMode = false;
    bCreate = false;
    bShowHidden = false;
    createPreview = NULL;
    lay = new QHBoxLayout;
    setLayout(lay);
    edtFile = new QLineEdit(this);
    lay->addWidget(edtFile);
    lay->addSpacing(3);
    lay->setMargin(0);
    lay->setSizeConstraint(QLayout::SetMinAndMaxSize);
    QPushButton *btnOpen = new QPushButton(this);
    lay->addWidget(btnOpen);
    btnOpen->setIcon(Icon("fileopen"));
    connect(btnOpen, SIGNAL(clicked()), this, SLOT(showFiles()));
    connect(edtFile, SIGNAL(textChanged(const QString&)), this, SLOT(editTextChanged(const QString&)));
}

EditFile::~EditFile()
{
}

void EditFile::editTextChanged(const QString &str)
{
    emit textChanged(str);
}

void EditFile::setText(const QString &t)
{
    edtFile->setText(t);
}

void EditFile::setFilter(const QString &f)
{
    filter = f;
}

void EditFile::setStartDir(const QString &d)
{
    startDir = d;
}

QString EditFile::text() const
{
    return edtFile->text();
}

void EditFile::setFilePreview(CreatePreview *preview)
{
    createPreview = preview;
}

void EditFile::setTitle(const QString &_title)
{
    title = _title;
}

void EditFile::setReadOnly(bool state)
{
    edtFile->setReadOnly(state);
}

void EditFile::setCreate(bool create)
{
    bCreate = create;
}

void EditFile::setShowHidden(bool value)
{
    bShowHidden = value;
}

void EditFile::showFiles()
{
	QString s = edtFile->text();
	if(bDirMode)
	{
		s = QFileDialog::getExistingDirectory(topLevelWidget(), title, s);
	}
	else if (bMultiplyMode)
	{
		QStringList lst = QFileDialog::getOpenFileNames(topLevelWidget(), QString::null, QString::null, filter);
		if ((lst.count() > 1) || ((lst.count() > 0) && (lst[0].indexOf(' ') >= 0)))
		{
			for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it)
			{
				*it ='\"' + *it + '\"';
			}
		}
		s = lst.join(" ");
	}
	else
	{
		if(s.isEmpty())
		{
			s = startDir;
			if (!s.isEmpty())
				makedir(s);
		}
		if(bCreate)
		{
			s = QFileDialog::getSaveFileName(topLevelWidget(), title, s, filter);
		}
		else
		{
			s = QFileDialog::getOpenFileName(topLevelWidget(), title, s, filter);
		}
	}
	if(s.length())
		edtFile->setText(QDir::convertSeparators(s));
}

EditSound::EditSound(QWidget *p)
        : EditFile(p)
{
    QPushButton *btnPlay = new QPushButton(this);
    lay->addWidget(btnPlay);
    btnPlay->setIcon(Icon("1rightarrow"));
    connect(btnPlay, SIGNAL(clicked()), this, SLOT(play()));

    filter = i18n("Sounds (*.wav *.mp3 *.flac *.ogg *.aiff *.mod *.s3m *.xm *.it)");

    startDir = app_file("sound");
    title = i18n("Select sound");
}

EditSound::~EditSound()
{
}

void EditSound::play()
{
    EventPlaySound e(edtFile->text());
    e.process();
}

/*
 * EditSoundDelegate
 */
EditSoundDelegate::EditSoundDelegate(int column, QObject *parent)
  : QItemDelegate(parent)
  , m_column(column)
{}

QWidget *EditSoundDelegate::createEditor(QWidget *parent,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    if(index.column() != m_column)
        return QItemDelegate::createEditor(parent, option, index);

    EditSound *es = new EditSound(parent);
    es->setText(index.model()->data(index, Qt::EditRole).toString());
    return es;
}

void EditSoundDelegate::setEditorData(QWidget *editor,
                                      const QModelIndex &index) const
{
    if(index.column() != m_column) {
        QItemDelegate::setEditorData(editor, index);
        return;
    }
    EditSound *es = static_cast<EditSound*>(editor);
    es->setText(index.model()->data(index, Qt::EditRole).toString());
}

void EditSoundDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                     const QModelIndex &index) const
{
    if(index.column() != m_column) {
        QItemDelegate::setModelData(editor, model, index);
        return;
    }
    EditSound *es = static_cast<EditSound*>(editor);
    model->setData(index, es->text(), Qt::EditRole);
}

void EditSoundDelegate::updateEditorGeometry(QWidget *editor,
                                             const QStyleOptionViewItem &option,
                                             const QModelIndex &/* index */) const
{
  editor->setGeometry(option.rect);
}

static QHash<QString, QByteArray> createHelpList(const char **helpList)
{
  QHash<QString, QByteArray> ret;
  for (const char **p = helpList; *p;) {
      const QString s = QString::fromUtf8(*p++);
      const QByteArray b = QByteArray(*p++);
      ret.insert(s, b);
  }
  return ret;
}

/*
 * LineEdit
 */
LineEdit::LineEdit(QWidget *parent)
        : QLineEdit(parent)
{}

void LineEdit::setHelpList(const QHash<QString, QByteArray> &helpList)
{
    m_helpList = helpList;
}

void LineEdit::setHelpList(const char **helpList)
{
  setHelpList(createHelpList(helpList));
}

void LineEdit::menuTriggered(QAction*a)
{
    insert(a->data().toString());
}

void LineEdit::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *popup = QLineEdit::createStandardContextMenu();
    connect(popup, SIGNAL(triggered(QAction*)), this, SLOT(menuTriggered(QAction*)));
    if (m_helpList.count()){
        popup->addSeparator();
        QHashIterator<QString, QByteArray> it(m_helpList);
        while(it.hasNext()) {
            it.next();
            QString s = it.key();
            s.replace('&', "&&");
            QString text = unquoteText(i18n(it.value().constData()));
            text += " (" + s + ")";

            QAction *a = new QAction(text, popup);
            a->setData(it.key());
            popup->addAction(a);
        }
    }
    popup->popup(e->globalPos());
    delete popup;
}

/*
 * LineEditDelegate
 */
LineEditDelegate::LineEditDelegate(int column, QObject *parent)
  : QItemDelegate(parent)
  , m_column(column)
{}

void LineEditDelegate::setHelpList(const QHash<QString, QByteArray> &helpList)
{
    m_helpList = helpList;
}

void LineEditDelegate::setHelpList(const char **helpList)
{
    setHelpList(createHelpList(helpList));
}

QWidget *LineEditDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    if(index.column() != m_column)
        return QItemDelegate::createEditor(parent, option, index);

    LineEdit *le = new LineEdit(parent);
    le->setText(index.model()->data(index, Qt::EditRole).toString());
    le->setHelpList(m_helpList);
    return le;
}

void LineEditDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
{
    if(index.column() != m_column) {
        QItemDelegate::setEditorData(editor, index);
        return;
    }
    LineEdit *le = static_cast<LineEdit*>(editor);
    le->setText(index.model()->data(index, Qt::EditRole).toString());
}

void LineEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    if(index.column() != m_column) {
        QItemDelegate::setModelData(editor, model, index);
        return;
    }
    LineEdit *le = static_cast<LineEdit*>(editor);
    model->setData(index, le->text(), Qt::EditRole);
}

void LineEditDelegate::updateEditorGeometry(QWidget *editor,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &/* index */) const
{
  editor->setGeometry(option.rect);
}

/*
 * MultiLineEdit
 */
MultiLineEdit::MultiLineEdit(QWidget *parent)
        : QTextEdit(parent)
{}

void MultiLineEdit::setHelpList(const QHash<QString, QByteArray> &helpList)
{
    m_helpList = helpList;
}

void MultiLineEdit::setHelpList(const char **helpList)
{
  setHelpList(createHelpList(helpList));
}

void MultiLineEdit::menuTriggered(QAction*a)
{
    insertPlainText(a->data().toString());
}

void MultiLineEdit::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *popup = QTextEdit::createStandardContextMenu();
    connect(popup, SIGNAL(triggered(QAction*)), this, SLOT(menuTriggered(QAction*)));
    if (m_helpList.count()){
        popup->addSeparator();
        QHashIterator<QString, QByteArray> it(m_helpList);
        while(it.hasNext()) {
            it.next();
            QString s = it.key();
            s = s.replace('&', "&&");

            QString text = unquoteText(i18n(it.value().constData()));
            text += " (" + s + ")";

            QAction *a = new QAction(text, popup);
            a->setData(it.key());
            popup->addAction(a);
        }
    }
    popup->exec(e->globalPos());
    delete popup;
}

// vim: set expandtab:
