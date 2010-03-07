/***************************************************************************
                          jabbersearch.cpp  -  description
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

#include <vector>

#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QTimer>
#include <QTabWidget>
#include <QObject>
#include <QRegExp>
#include <QCheckBox>
#include <QGridLayout>
#include <QPixmap>
#include <QVBoxLayout>

#include "simgui/ballonmsg.h"
#include "icons.h"
#include "misc.h"

#include "jabberclient.h"
#include "jabbersearch.h"
#include "jabber.h"
#include "jidadvsearch.h"

using namespace std;
using namespace SIM;

class CComboBox : public QComboBox
{
public:
    CComboBox(QWidget *parent, const char *name);
    void addItem(const QString &label, const QString &value);
    QString value();
protected:
    vector<QString> m_values;
};

CComboBox::CComboBox(QWidget *parent, const char *name)
        : QComboBox(parent)
{
    setObjectName(name);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
}

void CComboBox::addItem(const QString &label, const QString &value)
{
    m_values.push_back(value);
    insertItem(INT_MAX,label);
}

QString CComboBox::value()
{
    unsigned index = currentIndex();
    if (index >= m_values.size())
        return QString::null;
    return m_values[index];
}

const unsigned MAX_ELEMENTS = 8;

JabberSearch::JabberSearch(QWidget *parent, const char *name)
        : QWizardPage(parent)
		, m_vlay(NULL)
		, m_lay(NULL)
{
}

void JabberSearch::init(QWidget *receiver, JabberClient *client, const QString &jid, const QString &node, const QString &name, bool bRegister)
{
    m_client    = client;
    m_jid       = jid;
    m_node      = node;
    m_name		= name;
    m_receiver	= receiver;
    m_bXData	= false;
    m_bFirst	= true;
    m_bRegister = bRegister;
    m_bDirty	= false;
}

struct defFlds
{
    const char  *tag;
    const char  *name;
    bool        bRequired;
};

static defFlds fields[] =
    {
        { "username", I18N_NOOP("Username"), true },
        { "nick", I18N_NOOP("Nick"), false },
        { "email", I18N_NOOP("EMail"), false },
        { "first", I18N_NOOP("First Name"), false },
        { "last", I18N_NOOP("Last Name"), false },
        { "age_min", I18N_NOOP("Age min"), false },
        { "age_max", I18N_NOOP("Age max"), false },
        { "city", I18N_NOOP("City"), false },
        { NULL, NULL, false }
    };

void JabberSearch::addWidget(JabberAgentInfo *data)
{
    QWidget *widget = NULL;
    bool bJoin = false;
    if (!data->Type.str().isEmpty())
	{
        if (data->Type.str() == "x")
		{
            m_bXData = true;
            QWidget *w;
            foreach(w,m_widgets)
			{
                if(w) delete(w);
            }
            m_widgets.clear();
            foreach(w,m_labels)
			{
                if(w) delete(w);
            }
            m_labels.clear();
            foreach(w,m_descs)
			{
                if(w) delete(w);
            }
            m_descs.clear();
            m_instruction = QString::null;
        }
		else if (data->Type.str() == "title")
		{
            if (!data->Value.str().isEmpty())
                m_title = data->Value.str();
        }
		else if (data->Type.str() == "text-single")
		{
            widget = new QLineEdit(this);
            widget->setObjectName(data->Field.str());
            connect(widget, SIGNAL(returnPressed()), m_receiver, SLOT(search()));
            connect(widget, SIGNAL(textChanged(const QString&)), m_receiver, SLOT(textChanged(const QString&)));
            if (!data->Value.str().isEmpty())
                static_cast<QLineEdit*>(widget)->setText(data->Value.str());
        }
		else if (data->Type.str() == "text-private")
		{
            widget = new QLineEdit(this);
            widget->setObjectName(data->Field.str());
            static_cast<QLineEdit*>(widget)->setEchoMode(QLineEdit::Password);
            connect(widget, SIGNAL(returnPressed()), m_receiver, SLOT(search()));
            connect(widget, SIGNAL(textChanged(const QString&)), m_receiver, SLOT(textChanged(const QString&)));
            if (!data->Value.str().isEmpty())
                static_cast<QLineEdit*>(widget)->setText(data->Value.str());
        }
		else if (data->Type.str() == "text-multi")
		{
            widget = new QTextEdit(this);
            widget->setObjectName(data->Field.str());
            connect(widget, SIGNAL(returnPressed()), m_receiver, SLOT(search()));
            if (!data->Value.str().isEmpty())
                static_cast<QTextEdit*>(widget)->setText(data->Value.str());
        }
		else if (data->Type.str() == "boolean" && !data->Label.str().isEmpty())
		{
            widget = new QCheckBox(data->Label.str(), this);
            widget->setObjectName(qPrintable(data->Field.str()));
            if (!data->Value.str().isEmpty() && !data->Value.str().startsWith("0"))
                static_cast<QCheckBox*>(widget)->setChecked(true);
            data->Label.clear();
            bJoin = true;
        }
		else if (data->Type.str() == "fixed")
		{
            if (!data->Value.str().isEmpty())
			{
                QString text = i18(data->Value.str());
                text = text.replace(QRegExp("  +"), "\n");
                if (m_bFirst)
				{
                    if (!m_label.isEmpty())
                        m_label += '\n';
                    m_label += text;
                }
				else
				{
                    QLabel *label = new QLabel(text, this);
                    label->setWordWrap(true);
                    widget = label;
                    bJoin = true;
                }
            }
        }
		else if (data->Type.str() == "instructions")
		{
            if (!data->Value.str().isEmpty())
			{
                QString text = i18(data->Value.str());
                text = text.replace(QRegExp("  +"), "\n");
                if (!m_instruction.isEmpty())
                    m_instruction += '\n';
                m_instruction += text;
            }
        }
		else if (data->Type.str() == "list-single")
		{
            CComboBox *box = new CComboBox(this, qPrintable(data->Field.str()));
            int cur = 0;
            int n = 0;
            for (unsigned i = 0; i < data->nOptions.toULong(); i++){
                QString label = get_str(data->OptionLabels, i);
                QString val   = get_str(data->Options, i);
                if(!label.isEmpty() && !val.isEmpty())
				{
                    box->addItem(i18(label), val);
                    if (data->Value.str() == val)
                        cur = n;
                    n++;
                }
            }
            box->setCurrentIndex(cur);
            widget = box;
        }
		else if (data->Type.str() == "key")
		{
            if (!data->Value.str().isEmpty())
                m_key = data->Value.str();
        }
		else if (data->Type.str() == "password")
		{
            widget = new QLineEdit(this);
            widget->setObjectName("password");
            static_cast<QLineEdit*>(widget)->setEchoMode(QLineEdit::Password);
            connect(widget, SIGNAL(returnPressed()), m_receiver, SLOT(search()));
            connect(widget, SIGNAL(textChanged(const QString&)), m_receiver, SLOT(textChanged(const QString&)));
            data->Label.str() = "Password";
        }
		else if (data->Type.str() == "online")
		{
            widget = new QCheckBox(this);
            widget->setObjectName("online");
            static_cast<QCheckBox*>(widget)->setText(i18n("Online only"));
            bJoin = true;
        }
		else if (data->Type.str() == "sex")
		{
            CComboBox *box = new CComboBox(this, qPrintable(data->Field.str()));
            box->addItem(QString::null, "0");
            box->addItem(i18n("Male"), "1");
            box->addItem(i18n("Female"), "2");
            data->Label.str() == I18N_NOOP("Gender");
            widget = box;
        }
		else
		{
            defFlds *f;
            for (f = fields; f->tag; f++)
                if (data->Type.str() == QString::fromUtf8(f->tag))
                    break;
            if (f->tag)
			{
                widget = new QLineEdit(this);
                widget->setObjectName(f->tag);
                connect(widget, SIGNAL(returnPressed()), m_receiver, SLOT(search()));
                connect(widget, SIGNAL(textChanged(const QString&)), m_receiver, SLOT(textChanged(const QString&)));
                if (!data->Value.str().isEmpty())
                    static_cast<QLineEdit*>(widget)->setText(data->Value.str());
                data->Label.str() = QString::fromUtf8(f->name);
                if (f->bRequired && m_bRegister)
                    data->bRequired.asBool() = true;
            }
			else if (!data->Label.str().isEmpty())
			{
                widget = new QLineEdit(this);
                widget->setObjectName(data->Field.str());
                connect(widget, SIGNAL(returnPressed()), m_receiver, SLOT(search()));
                connect(widget, SIGNAL(textChanged(const QString&)), m_receiver, SLOT(textChanged(const QString&)));
                if (!data->Value.str().isEmpty())
                    static_cast<QLineEdit*>(widget)->setText(data->Value.str());
            }
        }
    }
	else
	{
        createLayout();
        m_widgets.clear();
        m_labels.clear();
        m_descs.clear();
        m_bDirty = true;
        QTimer::singleShot(0, this, SLOT(setSize()));
        return;
    }
    if (widget)
	{
        m_bFirst = false;
        if (data->bRequired.toBool())
            m_required.push_back(widget);
        QLabel *label = NULL;
        if (!bJoin && !data->Label.str().isEmpty())
		{
            QString text = i18(data->Label.str());
            if (!text.isEmpty() && (text[(int)(text.length() - 1)] != ':'))
                text += ':';
            label = new QLabel(text, this);
            label->setAlignment(Qt::AlignRight);
        }
        QWidget *help = NULL;
        if (!data->Desc.str().isEmpty())
            help = new HelpButton(data->Desc.str(), this);
        m_labels.push_back(label);
        m_widgets.push_back(widget);
        m_descs.push_back(help);
    }
}

void JabberSearch::setSize()
{
    if (!m_bDirty || (parent() == NULL))
        return;
    m_bDirty = false;
    for (QWidget *p = this; p; p = p->parentWidget()){
        QSize s  = p->sizeHint();
        QSize s1 = QSize(p->width(), p->height());
        if (s.isValid())
            p->setMinimumSize(s);
        p->resize(qMax(s.width(), s1.width()), qMax(s.height(), s1.height()));
        if (p->layout())
            p->layout()->invalidate();
        if (p == topLevelWidget())
            break;
    }
    QWidget *t = topLevelWidget();
    QSize s = t->sizeHint();
    t->resize(qMax(t->width(), s.width()), qMax(t->height(), s.height()));
    t->adjustSize();
}

#if 0
static const char *any_data[] =
    {
        I18N_NOOP("First (Given)"),
        I18N_NOOP("Last (Family)"),
        I18N_NOOP("Nick (Alias)"),
        I18N_NOOP("Email"),
        I18N_NOOP("Select the speed of the search.  \"Fast\" matches your string to the beginning of the field only (ie. \"b\" would yield Bob,Bill,etc...)  \"Slower\" matches your string anywhere in the field (ie. \"b\" would yield Bob, Bill, Caleb, Robbie, etc...)"),
        I18N_NOOP("Search Speed"),
        I18N_NOOP("Fast / Less accurate"),
        I18N_NOOP("Slower / More extensive")
        I18N_NOOP("Full name")
        I18N_NOOP("First Name")
        I18N_NOOP("Last Name")
        I18N_NOOP("Nickname")
        I18N_NOOP("E-mail")
        I18N_NOOP("Username")
        I18N_NOOP("Password")
        I18N_NOOP("Enter your MSN Messenger account and password. Example: user@hotmail.com.  Nickname is optional.")
        I18N_NOOP("Enter your AIM screenname or ICQ UIN and the password for that account")
        I18N_NOOP("Enter your YAHOO! Messenger Username and Password.")
        I18N_NOOP("Please enter your UIN and password")
        I18N_NOOP("You need a x:data capable client to register.")
        I18N_NOOP("Enter nick you want to register.")
        I18N_NOOP("Complete the form to submit your searchable attributes in the Jabber User Directory")
        I18N_NOOP("Fill in all of the fields to add yourself to the JUD.")
        I18N_NOOP("Fill in a field to search for any matching Jabber User (POSIX regular expressions allowed)")
        I18N_NOOP("Fill in a field to search for any matching Jabber users.")
        I18N_NOOP("To register, please fill out the following form.  Be as accurate as possible to make it easier for people to search for you.")
    };
#endif

QString JabberSearch::i18(const char *text)
{
    if ((text == NULL) || (*text == 0))
        return QString();
    return i18(QString::fromUtf8(text));
}

QString JabberSearch::i18(const QString &res)
{
    if (res.isEmpty())
        return QString();
    for (int i = 0; i < res.length(); i++){
        if (res[i].unicode() >= 0x80)
            return res;
    }
    return i18n(res);
}

bool JabberSearch::canSearch()
{
    bool bRes = true;

    QList<QLineEdit*> l = findChildren<QLineEdit*>();
    QLineEdit *edit;
    foreach(edit,l)
	{
		if (edit->echoMode() == QLineEdit::Password){
            if (edit->text().isEmpty())
                return false;
			continue;
		}
		if (edit->text().isEmpty()){
            foreach(QWidget *w, m_required){;
                if (w == (QWidget*)edit){
                    return false;
                }
			}
		}
		if (!edit->text().isEmpty())
			bRes = true;
	}
    return bRes;
}

QString JabberSearch::condition(QWidget *w)
{
    QString res;
    if (m_bXData && (w == NULL))
        res += "x:data";

    if (w == NULL)
        w = this;

    QList<QLineEdit*> list_edit = w->findChildren<QLineEdit*>();
    foreach( QLineEdit *edit, list_edit )
	{
        if (!edit->text().isEmpty()){
            if (!res.isEmpty())
                res += ';';
            res += edit->objectName();
            res += '=';
            res += quoteChars(edit->text(), ";");
        }
    }

    QList<QComboBox*> list_combo = w->findChildren<QComboBox*>();
    foreach( QComboBox *box, list_combo )
    {
        if (box->currentText().isEmpty()){
            continue;
        }
        if (!res.isEmpty())
            res += ';';
        res += box->objectName();
        res += '=';
        res += quoteChars(box->currentText(), ";");
    }

    QList<QCheckBox*> list_check = w->findChildren<QCheckBox*>();
    foreach( QCheckBox *check, list_check )
	{
        if (!res.isEmpty())
            res += ';';
        res += check->objectName();
        res += check->isChecked() ? "=1" : "=0";
    }

    QList<QTextEdit*> list_tedit = w->findChildren<QTextEdit*>();
    foreach( QTextEdit *edit, list_tedit )
	{
        if (!edit->toPlainText().isEmpty()){
            if (!res.isEmpty())
                res += ';';
            res += edit->objectName();
            res += '=';
            res += quoteChars(edit->toPlainText(), ";");
        }
    }

    if (!m_key.isEmpty() && (w == NULL)){
        if (!res.isEmpty())
            res += ';';
        res += "key=";
        res += quoteChars(m_key, ";");
    }
    return res;
}

void JabberSearch::createLayout()
{
    unsigned start = 0;
    unsigned nCols = 0;
    unsigned nRows = 0;
    m_vlay = new QVBoxLayout(this);
    m_lay = new QGridLayout(this);
    m_vlay->addLayout(m_lay);
    m_vlay->setMargin(11);
    m_lay->setSpacing(6);
    m_vlay->addStretch();
    if (!m_widgets.empty()){
        nCols = (m_widgets.size() + MAX_ELEMENTS - 1) / MAX_ELEMENTS;
        nRows = (m_widgets.size() + nCols - 1) / nCols;
        start = 0;
        if (!m_label.isEmpty())
		{
            QLabel *label = new QLabel(m_label, this);
            label->setWordWrap(true);
            m_lay->addWidget(label, 0, 0, 1, nCols * 3 + 1);
            m_label = QString::null;
            start = 1;
        }
        unsigned row = start;
        unsigned col = 0;
        for (int i = 0; i < m_widgets.size(); i++, row++)
		{
            if (row >= nRows + start){
                row  = 0;
                col += 3;
            }
            if (m_labels[i])
			{
                static_cast<QLabel*>(m_labels[i])->setAlignment( Qt::AlignVCenter | Qt::AlignRight);
                m_lay->addWidget(m_labels[i], row, col);
                if (m_descs[i])
				{
                    m_lay->addWidget(m_widgets[i], row, col + 1, Qt::AlignVCenter);
                    m_lay->addWidget(m_descs[i], row, col + 2, Qt::AlignVCenter);
                    m_descs[i]->show();
                }
				else
				{
                    m_lay->addWidget(m_widgets[i], row, col + 1, 1, 1, Qt::AlignVCenter);
                }
                m_labels[i]->show();
            }
			else
			{
                if (m_descs[i])
				{
                    m_lay->addWidget(m_widgets[i], row, col, 1, 2, Qt::AlignVCenter);
                    m_lay->addWidget(m_descs[i], row, col + 2, Qt::AlignBottom);
                    m_descs[i]->show();
                }
				else
                    m_lay->addWidget(m_widgets[i], row, col, 1, 3, Qt::AlignVCenter);

            }
            m_widgets[i]->show();
        }
    }
    if (!m_instruction.isEmpty())
	{
        QLabel *label = new QLabel(m_instruction, this);
        label->setWordWrap(true);
        m_lay->addWidget(label, nRows + start, 0, 1, nCols * 3 - 1);
        m_instruction = QString::null;
    }
}

HelpButton::HelpButton(const QString &help, QWidget *parent)
        : QPushButton(parent)
{
    QIcon p = Icon("help");
    setIcon(p);
    m_help = help;
    connect(this, SIGNAL(clicked()), this, SLOT(click()));
//    setMinimumSize(p.width() + 2, p.height() + 2);
//    setMaximumSize(p.width() + 2, p.height() + 2);
}

void HelpButton::click()
{
    BalloonMsg::message(m_help, this);
}

JIDJabberSearch::JIDJabberSearch(QWidget *parent, const char *name)
        : JabberSearch(parent, name)
{
    m_adv = NULL;
}

void JIDJabberSearch::setAdvanced(JIDAdvSearch *adv)
{
    m_adv = adv;
}

static const char *names[] =
    {
        "username",
        "email",
        "nick",
        "first",
        "last",
        "user",
        "fn",
        "given",
        "online",
        NULL
    };

const unsigned MAX_MAIN	= 6;

void JIDJabberSearch::createLayout()
{
    unsigned rowMain = 0;
    QGridLayout *lay = new QGridLayout(this);
    QGridLayout *alay = NULL;
    lay->setSpacing(6);
    unsigned nAdv = 0;
    unsigned nMain = 0;
    if (m_widgets.size() > (int)MAX_MAIN){
        alay = new QGridLayout(m_adv->grpSearch);
        alay->setMargin(11);
        alay->setSpacing(6);
        for (int i = 0; i < m_widgets.size(); i++){
            if (nMain > MAX_MAIN){
                nAdv++;
                continue;
            }
            const char **p;
            for (p = names; *p; p++)
                if ( m_widgets[i]->objectName() == *p )
                    break;
            if (*p == NULL){
                nAdv++;
                continue;
            }
            nMain++;
        }
    }
    unsigned nCols = (nAdv + MAX_ELEMENTS - 1) / MAX_ELEMENTS;
    unsigned nRows = nCols ? (nAdv + nCols - 1) / nCols : 0;
    unsigned start = 0;
    unsigned row   = 0;
    unsigned col   = 0;
    nMain = 0;
    for (int i = 0; i < m_widgets.size(); i++){
        bool bMain = false;
        if (alay){
            if (nMain < MAX_MAIN){
                const char **p;
                for (p = names; *p; p++)
                    if ( m_widgets[i]->objectName() == *p )
                        break;
                if (*p){
                    nMain++;
                    bMain = true;
                }
            }
        }else{
            bMain = true;
        }
        if (bMain){
            if (m_labels[i]){
                static_cast<QLabel*>(m_labels[i])->setAlignment(Qt::AlignVCenter);
                lay->addWidget(m_labels[i], rowMain, 0, 1, 2, Qt::AlignVCenter);
                m_labels[i]->show();
                rowMain++;
            }
            if (m_descs[i]){
                lay->addWidget(m_widgets[i], rowMain, 0, Qt::AlignVCenter);
                lay->addWidget(m_descs[i], rowMain, 1, Qt::AlignVCenter);
                m_descs[i]->show();
            }else{
                lay->addWidget(m_widgets[i], rowMain, 0, 1, 2, Qt::AlignVCenter);
            }
            m_widgets[i]->show();
            rowMain++;
        }else{
            if (row >= nRows + start){
                row  = 0;
                col += 3;
            }
            m_widgets[i]->setParent(m_adv->grpSearch);
            m_widgets[i]->move(QPoint(0, 0));
            if (m_descs[i]){
                m_descs[i]->setParent(m_adv->grpSearch);
                m_descs[i]->move(QPoint(0, 0));
            }
            if (m_labels[i]){
                m_labels[i]->setParent(m_adv->grpSearch);
                m_labels[i]->move(QPoint(0, 0));
                static_cast<QLabel*>(m_labels[i])->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
                alay->addWidget(m_labels[i], row, col);
                if (m_descs[i]){
                    alay->addWidget(m_widgets[i], row, col + 1, Qt::AlignVCenter);
                    alay->addWidget(m_descs[i], row, col + 2, Qt::AlignVCenter);
                    m_descs[i]->show();
                }else{
                    alay->addWidget(m_widgets[i], row, col + 1, 1, 2, Qt::AlignVCenter);
                }
                m_labels[i]->show();
            }else{
                if (m_descs[i]){
                    alay->addWidget(m_widgets[i], row, col, 1, 2, Qt::AlignVCenter);
                    alay->addWidget(m_descs[i], row, col + 2, Qt::AlignBottom);
                    m_descs[i]->show();
                }else{
                    alay->addWidget(m_widgets[i], row, col, 1, 3, Qt::AlignVCenter);
                }
            }
            m_widgets[i]->show();
            row++;
        }
    }
    if (alay){
        m_adv->lblTitle->setText(m_title);
        m_adv->lblInstruction->setText(m_instruction);
    }
    m_instruction = QString::null;
}

/*
#ifndef NO_MOC_INCLUDES
#include "jabbersearch.moc"
#endif
*/

