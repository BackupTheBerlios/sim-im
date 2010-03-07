/***************************************************************************
                          spell.cpp  -  description
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

#include "spell.h"
#include "spellcfg.h"
#include "speller.h"
#include "spellhighlight.h"
#include "core.h"

#include "profile.h"
#include "profilemanager.h"

#include <QApplication>
#include <QWidget>
#include <QFile>
#include <QByteArray>
#include <QChildEvent>
#include <QEvent>
#include <algorithm>

using namespace std;
using namespace SIM;

class PSpellHighlighter : public SpellHighlighter
{
public:
    PSpellHighlighter(QTextEdit *edit, SpellPlugin *plugin);
    ~PSpellHighlighter();
};

PSpellHighlighter::PSpellHighlighter(QTextEdit *edit, SpellPlugin *plugin)
    : SpellHighlighter(edit, plugin)
{
}

PSpellHighlighter::~PSpellHighlighter()
{
}

SIM::Plugin *createSpellPlugin(unsigned base, bool, Buffer *config)
{
    SIM::Plugin *plugin = new SpellPlugin(base, config);
    return plugin;
}

static SIM::PluginInfo info =
    {
        I18N_NOOP("Spell check"),
        I18N_NOOP("Plugin provides check spelling"),
        VERSION,
        createSpellPlugin,
        SIM::PLUGIN_DEFAULT
    };

EXPORT_PROC SIM::PluginInfo* GetPluginInfo()
{
    return &info;
}

SpellPlugin::SpellPlugin(unsigned base, Buffer *config)
  : QObject(), Plugin(base)
  , EventReceiver()
  , m_bActive (false)
  , m_base (NULL)
{
    m_propertyHub = SIM::PropertyHub::create("replace");

    CmdSpell = registerType();

    SIM::Command cmd;
    cmd->id         = CmdSpell;
    cmd->text       = "_";
    cmd->menu_id    = MenuTextEdit;
    cmd->menu_grp   = 0x0100;
    cmd->flags      = SIM::COMMAND_CHECK_STATE;
    EventCommandCreate(cmd).process();
}

SpellPlugin::~SpellPlugin()
{
    EventCommandRemove(CmdSpell).process();
    deactivate();
    while( !m_spellers.empty() ) {
        delete m_spellers.first();
        m_spellers.removeFirst();
    }
    delete m_base;
}

void SpellPlugin::reset()
{
    while( !m_spellers.empty() ) {
        delete m_spellers.first();
        m_spellers.removeFirst();
    }
    if (m_base)
        delete m_base;
#ifdef WIN32
    m_base = new SpellerBase(value("Path").toString());
#else
    m_base = new SpellerBase(QString());
#endif
    SpellerConfig cfg(*m_base);
    QString ll = value("Lang").toString();
    while (!ll.isEmpty()){
        QString l = SIM::getToken(ll, ';');
        cfg.setKey("lang", l);
        cfg.setKey("encoding", "utf-8");
        Speller *speller = new Speller(&cfg);
        if (speller->created()){
            m_spellers.push_back(speller);
            continue;
        }
        delete speller;
    }
    if (m_spellers.empty()){
        deactivate();
    }else{
        activate();
    }
    configChanged();
}

void SpellPlugin::activate()
{
    if( m_bActive )
        return;
    m_bActive = true;
    qApp->installEventFilter(this);
    QWidgetList list = QApplication::allWidgets();
    foreach( QWidget *w, list ) {
        if (w->inherits("TextEdit"))
            new PSpellHighlighter(static_cast<QTextEdit*>(w), this);
    }
}

void SpellPlugin::deactivate()
{
    if (!m_bActive)
        return;
    m_bActive = false;
    qApp->removeEventFilter(this);
}

QByteArray SpellPlugin::getConfig()
{
    return QByteArray();
}

QWidget *SpellPlugin::createConfigWindow(QWidget *parent)
{
    return new SpellConfig(parent, this);
}

bool SpellPlugin::processEvent(SIM::Event* e)
{
    if(e->type() == eEventPluginLoadConfig)
	{
        PropertyHubPtr hub = ProfileManager::instance()->getPropertyHub("spell");
        if(!hub.isNull())
            setPropertyHub(hub);
        reset();
    }
    return false;
}

class NewChildEvent : public QEvent
{
public:
    NewChildEvent() : QEvent( (QEvent::Type)( QEvent::User + 100 ) ) {};
};

bool SpellPlugin::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::ChildAdded){
        QChildEvent *ce = static_cast<QChildEvent*>(e);
        QObject *pChild = ce->child();
        connect( pChild, SIGNAL(destroyed(QObject*)), SLOT(tempChildDestroyed(QObject*)) );
        m_listTempChilds.push_back( pChild );
        QApplication::postEvent( this, new NewChildEvent() );
    }
    return QObject::eventFilter(o, e);
}

void SpellPlugin::tempChildDestroyed( QObject *pObject ) {
    int index = m_listTempChilds.indexOf( pObject );
    if( -1 != index ) {
        m_listTempChilds.removeAt( index );
        pObject->disconnect( this );
    }
}

bool SpellPlugin::event( QEvent *e ) {
    if( ( e->type() == ( QEvent::User + 100 ) ) && ( m_listTempChilds.count() > 0 ) ) {
        QObject *pChild = m_listTempChilds.first();
        pChild->disconnect( this );
        m_listTempChilds.pop_front();
        if( pChild->inherits( "MsgTextEdit" ) ) {
            QTextEdit *edit = static_cast<QTextEdit*>( pChild );
            new PSpellHighlighter(edit, this);
        }
        return true;
    }
    QObject::event( e );
    return false;
}

void SpellPlugin::textEditFinished(QTextEdit *edit)
{
}

bool SpellPlugin::check(const QString &word)
{
    if( -1 != m_listIgnore.indexOf( word ) )
        return true;

    foreach( Speller *pSpeller, m_spellers ) {
        if( pSpeller->check(word.toUtf8()) == 1 )
            return true;
    }

    return false;
}

void SpellPlugin::add(const QString &word)
{
    foreach( Speller *pSpeller, m_spellers ) {
        if( pSpeller->add(word.toUtf8()) )
            return;
    }
}

void SpellPlugin::ignore(const QString &word) {
    if( -1 != m_listIgnore.indexOf( word ) )
        m_listIgnore.push_back( word );
}

struct WordWeight
{
    QString     word;
    unsigned    weight;
};

bool operator < (const WordWeight &w1, const WordWeight &w2) { return w1.weight > w2.weight; }

static unsigned weight(const QString &s1, const QString &s2)
{
    QString s = s2;
    unsigned res = 0;
    for (int i = 0; i < (int)(s1.length()); i++){
        for (int j = 0; j < (int)(s.length()); j++){
            if (s1[i] == s[j]){
                s = s.left(j) + s.mid(j + 1);
                res++;
                break;
            }
        }
    }
    return res;
}

QStringList SpellPlugin::suggestions(const QString &word)
{
    QStringList res;
    foreach( Speller *pSpeller, m_spellers ) {
        QStringList wl = pSpeller->suggestions(word.toUtf8());
        for (QStringList::Iterator it = wl.begin(); it != wl.end(); ++it){
            QString wrd = (*it);
            QStringList::Iterator itr;
            for (itr = res.begin(); itr != res.end(); ++itr){
                if ((*itr) == wrd)
                    break;
            }
            if (itr == res.end())
                res.append(wrd);
        }
    }
    std::vector<WordWeight> words;
    for (QStringList::Iterator itw = res.begin(); itw != res.end(); ++itw){
        unsigned w = weight(word, *itw);
        if (w == 0)
            continue;
        WordWeight ww;
        ww.word   = *itw;
        ww.weight = w;
        words.push_back(ww);
    }
	sort(words.begin(), words.end());
    unsigned size = words.size();
    if (size > 15)
        size = 15;
    res.clear();
    for (unsigned i = 0; i < size; i++)
        res.append(words[i].word);
    return res;
}

void SpellPlugin::setPropertyHub(SIM::PropertyHubPtr hub)
{
	m_propertyHub = hub;
}

SIM::PropertyHubPtr SpellPlugin::propertyHub()
{
	return m_propertyHub;
}

QVariant SpellPlugin::value(const QString& key)
{
	return m_propertyHub->value(key);
}

void SpellPlugin::setValue(const QString& key, const QVariant& v)
{
	m_propertyHub->setValue(key, v);
}
