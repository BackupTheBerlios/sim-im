/***************************************************************************
                          spellhighlight.cpp  -  description
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

#include "log.h"
#include "simgui/textshow.h"

#include "spellhighlight.h"
#include "spell.h"
#include "msgedit.h"

#include <QTimer>
#include <QByteArray>

using namespace std;
using namespace SIM;
const unsigned ErrorColor = 0xFF0101;

SpellHighlighter::SpellHighlighter(QTextEdit *edit, SpellPlugin *plugin)
    : QSyntaxHighlighter(edit)
    , EventReceiver(SIM::HighPriority)
    , m_plugin( plugin )
{
}

SpellHighlighter::~SpellHighlighter()
{
}

void SpellHighlighter::highlightBlock( const QString &sText )
{
    QTextCharFormat format;
    format.setUnderlineColor( Qt::red );
    format.setUnderlineStyle( QTextCharFormat::SpellCheckUnderline );

    QRegExp expression( "\\b(\\w+)\\W+" );
    QRegExp expression2( "\\b(\\w+)" );
    int index = expression.indexIn( sText );
    while (index >= 0) {
        int length = expression.matchedLength();
        QString s = sText.mid( index, length );
        expression2.indexIn( s );
        QString sWord = s.left( expression2.matchedLength() );
        if( !m_plugin->check( sWord ) ) {
            setFormat( index, expression2.matchedLength(), format );
        }
        index = expression.indexIn( sText, index + length );
    }
}

bool SpellHighlighter::processEvent(SIM::Event *e)
{
    if (e->type() == SIM::eEventCheckCommandState){
        SIM::EventCheckCommandState *ecs = static_cast<SIM::EventCheckCommandState*>(e);
        SIM::CommandDef *cmd = ecs->cmd();
        if (cmd->id == m_plugin->CmdSpell){
            TextEdit *pEdit = (TextEdit*)(cmd->param);
            if( pEdit->document() != document() )
                return false;
            m_listSuggestions.clear();
            QTextCursor cursor = pEdit->cursorForPosition( pEdit->m_popupPos );
            cursor.select( QTextCursor::WordUnderCursor );
            QString sWord = cursor.selectedText();
            if( sWord.isEmpty() )
                return false;
            if( m_plugin->check( sWord ) )
                return false;

            m_listSuggestions = m_plugin->suggestions( sWord );
            SIM::CommandDef *cmds = new SIM::CommandDef[m_listSuggestions.count() + 3];
            unsigned i = 0;
            for (QStringList::Iterator it = m_listSuggestions.begin(); it != m_listSuggestions.end(); ++it, i++){
                cmds[i].id   = m_plugin->CmdSpell + i + 2;
                cmds[i].text = "_";
                cmds[i].text_wrk = (*it);
                if (i >= 10){
                    i++;
                    break;
                }
            }
            cmds[i].id   = m_plugin->CmdSpell;
            cmds[i].text = "_";
            cmds[i].text_wrk = i18n("Add '%1'").arg( sWord );
            i++;
            cmds[i].id   = m_plugin->CmdSpell + 1;
            cmds[i].text = "_";
            cmds[i].text_wrk = i18n("Ignore '%1'").arg( sWord );

            cmd->param  = cmds;
            cmd->flags |= SIM::COMMAND_RECURSIVE;

            return true;
        }
    } else
    if (e->type() == eEventCommandExec){
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if ((cmd->id >= m_plugin->CmdSpell) && (cmd->id < m_plugin->CmdSpell + m_listSuggestions.count() + 1)){
            TextEdit *pEdit = (TextEdit*)(cmd->param);
            if( pEdit->document() != document() )
                return false;
            QTextCursor cursor = pEdit->cursorForPosition( pEdit->m_popupPos );
            cursor.select( QTextCursor::WordUnderCursor );
            QString sWord = cursor.selectedText();
            if( sWord.isEmpty() )
                return false;
            if (cmd->id == m_plugin->CmdSpell){
                m_plugin->add( sWord );
/*
                MAP_BOOL::iterator it = m_words.find(SIM::my_string( sWord ));
                if (it == m_words.end()){
                    m_words.insert(MAP_BOOL::value_type(SIM::my_string(m_word), true));
                }else{
                    if (it->second)
                        return false;
                    it->second = true;
                }
                m_bDirty = true;
                QTimer::singleShot(300, this, SLOT(reformat()));
*/
            }else  if (cmd->id == m_plugin->CmdSpell + 1){
/*
                MAP_BOOL::iterator it = m_plugin->m_ignore.find(SIM::my_string(m_word));
                if (it == m_plugin->m_ignore.end())
                    m_plugin->m_ignore.insert(MAP_BOOL::value_type(SIM::my_string(m_word), true));
                it = m_words.find(SIM::my_string(m_word));
                if (it == m_words.end()){
                    m_words.insert(MAP_BOOL::value_type(SIM::my_string(m_word), true));
                }else{
                    if (it->second)
                        return false;
                    it->second = true;
                }
                m_bDirty = true;
                QTimer::singleShot(300, this, SLOT(reformat()));
*/
            }else{
                sWord = m_listSuggestions[cmd->id - m_plugin->CmdSpell - 2];
                cursor.insertText( sWord );
            }
        }
    }

    return false;
}
