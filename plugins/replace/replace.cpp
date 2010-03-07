/***************************************************************************
                          replace.cpp  -  description
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

#include "replace.h"
#include "replacecfg.h"
#include "simgui/textshow.h"
#include "html.h"
#include "misc.h"

#include "profile.h"
#include "profilemanager.h"

#include <QApplication>
#include <QKeyEvent>

using namespace std;
using namespace SIM;

Plugin *createReplacePlugin(unsigned base, bool, Buffer *cfg)
{
    Plugin *plugin = new ReplacePlugin(base, cfg);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Replace text"),
        I18N_NOOP("Plugin provides text replacing in message edit window"),
        VERSION,
        createReplacePlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}


ReplacePlugin::ReplacePlugin(unsigned base, Buffer *cfg)
  : QObject(), Plugin(base)
  , EventReceiver()
{
    m_propertyHub = SIM::PropertyHub::create("replace");
    qApp->installEventFilter(this);
}

ReplacePlugin::~ReplacePlugin()
{

}

QByteArray ReplacePlugin::getConfig()
{
    return QByteArray();
}

QWidget *ReplacePlugin::createConfigWindow(QWidget *parent)
{
    return new ReplaceCfg(parent, this);
}

class _UnquoteParser : public HTMLParser
{
public:
    _UnquoteParser(const QString &text);
    QString m_text;
protected:
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);
};

_UnquoteParser::_UnquoteParser(const QString &text)
{
    parse(text);
}

void _UnquoteParser::text(const QString &text)
{
    m_text += text;
}

void _UnquoteParser::tag_start(const QString &tag, const list<QString>&)
{
    if (tag == "img")
        m_text += ' ';
    if (tag == "br")
        m_text += '\n';
}

void _UnquoteParser::tag_end(const QString&)
{
}

bool ReplacePlugin::eventFilter(QObject *o, QEvent *e)
{
    if ((e->type() == QEvent::KeyPress) && o->inherits("MsgTextEdit")){
        QKeyEvent *ke = (QKeyEvent*)e;
        if ((ke->key() == Qt::Key_Enter) || (ke->key() == Qt::Key_Return) || (ke->key() == Qt::Key_Space)){
/*
            TextEdit *edit = (TextEdit*)o;
            int paraFrom, paraTo, indexFrom, indexTo;
            edit->getSelection(&paraFrom, &indexFrom, &paraTo, &indexTo);
            if ((paraFrom == paraTo) && (indexFrom == indexTo)){
                int parag, index;
                edit->getCursorPosition(&parag, &index);
                _UnquoteParser p(edit->text(parag));
                QString text = p.m_text.left(index);
                for (unsigned i = 1; i <= value("Keys").toUInt(); i++){
                    QString key = value("Key").toStringList().value(i);
                    if (key.length() > text.length())
                        continue;
                    if (key != text.mid(text.length() - key.length()))
                        continue;
                    if ((key.length() < text.length()) && !text[(int)(text.length() - key.length() - 1)].isSpace())
                        continue;
                    edit->setSelection(parag, index - key.length(), parag, index, 0);
                    edit->insert(value("Value").toStringList().value(i), false, false);
                    break;
                }
            }
*/
        }
    }
    return QObject::eventFilter(o, e);
}

bool ReplacePlugin::processEvent(SIM::Event *e)
{
	if(e->type() == eEventPluginLoadConfig)
	{
                PropertyHubPtr hub = ProfileManager::instance()->getPropertyHub("replace");
        if(!hub.isNull())
            setPropertyHub(hub);
	}
    return false;
}

void ReplacePlugin::setPropertyHub(SIM::PropertyHubPtr hub)
{
	m_propertyHub = hub;
}

SIM::PropertyHubPtr ReplacePlugin::propertyHub()
{
	return m_propertyHub;
}

QVariant ReplacePlugin::value(const QString& key)
{
	return m_propertyHub->value(key);
}

void ReplacePlugin::setValue(const QString& key, const QVariant& v)
{
	m_propertyHub->setValue(key, v);
}
