/***************************************************************************
                          cmddef.cpp  -  description
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

#include "cmddef.h"

#include <list>
#include <map>

#include "event.h"
#include "log.h"
#include "misc.h"

namespace SIM
{

using namespace std;

Command::Command()
{
    m_cmd.clear();
}

CommandDef &Command::operator=(const CommandDef &cmd)
{
    m_cmd = cmd;
    return m_cmd;
}

class CommandsDefPrivate : public EventReceiver
{
public:
    CommandsDefPrivate(unsigned id, bool bMenu);
    void setConfig(const QString &cfg);
    virtual bool processEvent(Event *e);
    bool addCommand(const CommandDef *cmd);
    bool changeCommand(const CommandDef *cmd);
    bool delCommand(unsigned id);
    void generateConfig();
    list<CommandDef> buttons;
    list<unsigned> cfg;
    QString config;
    unsigned m_id;
    bool m_bMenu;
};

CommandsDefPrivate::CommandsDefPrivate(unsigned id, bool bMenu)
{
    m_id = id;
    m_bMenu = bMenu;
}

bool CommandsDefPrivate::changeCommand(const CommandDef *cmd)
{
    list<CommandDef>::iterator it;
    for (it = buttons.begin(); it != buttons.end(); ++it)
	{
        if (it->id == cmd->id)
		{
            (*it) = *cmd;
            return true;
        }
    }
    return false;
}

bool CommandsDefPrivate::addCommand(const CommandDef *cmd)
{
    if (changeCommand(cmd))
        return false;
    unsigned item_grp = m_bMenu ? cmd->menu_grp : cmd->bar_grp;
    if (item_grp)
	{
        list<CommandDef>::iterator it;
        for (it = buttons.begin(); it != buttons.end(); ++it)
		{
            unsigned grp = m_bMenu ? it->menu_grp : it->bar_grp;
            if (grp > item_grp){
                buttons.insert(it, *cmd);
                return true;
            }
        }
    }
    buttons.push_back(*cmd);
    return true;
}

bool CommandsDefPrivate::delCommand(unsigned id)
{
    for (list<CommandDef>::iterator it = buttons.begin(); it != buttons.end(); ++it){
        if (it->id == id){
            buttons.erase(it);
            return true;
        }
    }
    return false;
}

bool CommandsDefPrivate::processEvent(Event *e)
{
	list<CommandDef>::iterator it;
	switch (e->type()){
		case eEventCommandCreate:
			{
				EventCommandCreate *ecc = static_cast<EventCommandCreate*>(e);
				CommandDef *def = ecc->cmd();
				if(((m_bMenu ? def->menu_id : def->bar_id) == m_id) && (m_bMenu || !def->icon.isEmpty())){
					if (addCommand(def))
						cfg.clear();
				}
				break;
			}
		case eEventCommandChange:
			{
				EventCommandChange *ecc = static_cast<EventCommandChange*>(e);
				CommandDef *def = ecc->cmd();
				if (def->param == NULL){
					for (it = buttons.begin(); it != buttons.end(); ++it){
						if (it->id == def->id){
							*it = *def;
							break;
						}
					}
				}
				break;
			}
		case eEventCommandRemove:
			{
				EventCommandRemove *ecr = static_cast<EventCommandRemove*>(e);
				if (delCommand(ecr->id()))
					cfg.clear();
				break;
			}
		default:
			break;
	}
	return false;
}

void CommandsDefPrivate::setConfig(const QString &cfg_str)
{
    if (cfg_str == config && cfg.size())
        return;
    cfg.clear();
    config = cfg_str;
    generateConfig();
}

void CommandsDefPrivate::generateConfig()
{
    if(cfg.size())
        return;
    if(config.length())
	{
        list<unsigned> processed;
        QString active = config;
        QString noactive;
        int n = config.indexOf('/');
        if (n >= 0){
            active   = config.left(n);
            noactive = config.mid(n + 1);
        }
        while (active.length())
		{
            QString v = getToken(active, ',');
            unsigned id = v.toUInt();
            cfg.push_back(id);
            if (id)
                processed.push_back(id);
        }
        while (noactive.length())
		{
            QString v = getToken(noactive, ',');
            unsigned id = v.toUInt();
            if (id)
                processed.push_back(id);
        }
        for (list<CommandDef>::iterator it = buttons.begin(); it != buttons.end(); ++it)
		{
            CommandDef &c = (*it);
            unsigned grp = m_bMenu ? c.menu_grp : c.bar_grp;
            if (grp == 0)
                continue;
            list<unsigned>::iterator it_p;
            for (it_p = processed.begin(); it_p != processed.end(); ++it_p)
                if ((*it_p) == c.id)
                    break;
            if (it_p != processed.end())
                continue;
            unsigned cur_grp = 0;
            for (it_p = cfg.begin(); it_p != cfg.end(); ++it_p){
                if ((*it_p) == 0){
                    if (cur_grp == grp)
                        break;
                    continue;
                }
                list<CommandDef>::iterator itl;
                for (itl = buttons.begin(); itl != buttons.end(); ++itl)
                    if ((*itl).id == (*it_p))
                        break;
                if (itl == buttons.end())
                    continue;
                unsigned itl_grp = m_bMenu ? (*itl).menu_grp : (*itl).bar_grp;
                if (itl_grp == 0)
                    continue;
                cur_grp = itl_grp;
                if (grp > cur_grp)
                    break;
            }
            cfg.insert(it_p, c.id);
        }
    }
	else
	{
        unsigned cur_grp = 0;
        for (list<CommandDef>::iterator it = buttons.begin(); it != buttons.end(); ++it)
		{
            CommandDef &c = (*it);
            unsigned grp = m_bMenu ? c.menu_grp : c.bar_grp;
            if (grp == 0)
                continue;
            if ((grp & ~0xFF) != (cur_grp & ~0xFF))
			{
                if (cur_grp)
                    cfg.push_back(0);
            }
            cur_grp = grp;
            cfg.push_back(c.id);
        }
    }
}

class CommandsListPrivate
{
public:
    CommandsListPrivate(CommandsDefPrivate *def);
    virtual ~CommandsListPrivate() {}
    virtual CommandDef *next() = 0;
    virtual void reset() = 0;
    CommandsDefPrivate *m_def;
};

CommandsListPrivate::CommandsListPrivate(CommandsDefPrivate *def)
{
    m_def = def;
}

class CommandsListPrivateFull : public CommandsListPrivate
{
public:
    CommandsListPrivateFull(CommandsDefPrivate *p);
    list<CommandDef>::iterator it;
    virtual CommandDef *next();
    virtual void reset();
};

CommandsListPrivateFull::CommandsListPrivateFull(CommandsDefPrivate *def)
        : CommandsListPrivate(def)
{
    reset();
}

void CommandsListPrivateFull::reset()
{
    it = m_def->buttons.begin();
}

CommandDef *CommandsListPrivateFull::next()
{
    if (it == m_def->buttons.end())
        return NULL;
    CommandDef *res = &(*it);
    ++it;
    return res;
}

class CommandsListPrivateShort : public CommandsListPrivate
{
public:
    CommandsListPrivateShort(CommandsDefPrivate *p);
    list<unsigned>::iterator it;
    virtual CommandDef *next();
    virtual void reset();
};

CommandsListPrivateShort::CommandsListPrivateShort(CommandsDefPrivate *def)
        : CommandsListPrivate(def)
{
    reset();
}

void CommandsListPrivateShort::reset()
{
    it = m_def->cfg.begin();
}

static CommandDef SeparatorDef = CommandDef ();

CommandDef *CommandsListPrivateShort::next()
{
	for (;;++it)
	{
		if (it == m_def->cfg.end())
			return NULL;
		unsigned id = (*it);
		if (id == 0){
			++it;
			return &SeparatorDef;
		}
		for (list<CommandDef>::iterator itb = m_def->buttons.begin(); itb != m_def->buttons.end(); ++itb){
			if ((*itb).id == id){
				++it;
				return &(*itb);
			}
		}
	}
}

CommandsList::CommandsList(CommandsDef &def, bool bFull)
{
    def.p->generateConfig();
    if (bFull)
        p = new CommandsListPrivateFull(def.p);
	else
        p = new CommandsListPrivateShort(def.p);
}

CommandsList::~CommandsList()
{
    delete p;
}

CommandDef *CommandsList::operator ++()
{
    CommandDef *c = p->next();
    return c;
}

void CommandsList::reset()
{
    p->reset();
}

CommandsDef::CommandsDef(unsigned id, bool bMenu)
{
    p = new CommandsDefPrivate(id, bMenu);
}

CommandsDef::~CommandsDef()
{
    delete p;
}

unsigned CommandsDef::id()
{
    return p->m_id;
}

bool CommandsDef::isMenu()
{
    return p->m_bMenu;
}

void CommandsDef::set(const CommandDef *cmd)
{
    p->changeCommand(cmd);
}

void CommandsDef::set(const CommandDef &cmd)
{
    p->changeCommand(&cmd);
}

void CommandsDef::setConfig(const QString &cfg_str)
{
    p->setConfig(cfg_str);
}

void CommandsDef::setConfig(const QVariant &cfg_variant)
{
    p->setConfig(cfg_variant.toString());
}

class CommandsMapPrivate : public map<unsigned, CommandDef>
{
};

CommandsMap::CommandsMap()
{
    p = new CommandsMapPrivate;
}

CommandsMap::~CommandsMap()
{
    delete p;
}

CommandDef *CommandsMap::find(unsigned id)
{
    CommandsMapPrivate::iterator it = p->find(id);
    if (it == p->end())
        return NULL;
    return &it->second;
}

bool CommandsMap::add(CommandDef *def)
{
    CommandsMapPrivate::iterator it = p->find(def->id);
    if (it == p->end()){
        p->insert(CommandsMapPrivate::value_type(def->id, *def));
        return true;
    }
    it->second = *def;
    return false;
}

bool CommandsMap::erase(unsigned id)
{
    CommandsMapPrivate::iterator it = p->find(id);
    if (it == p->end())
        return false;
    p->erase(it);
    return true;
}

void CommandsMap::clear()
{
    p->clear();
}

class CommandsMapIteratorPrivate
{
    COPY_RESTRICTED(CommandsMapIteratorPrivate)
public:
CommandsMapIteratorPrivate(CommandsMapPrivate &_map) : map(_map)
    { it = map.begin(); }
    CommandsMapPrivate::iterator it;
    CommandsMapPrivate &map;
};

CommandsMapIterator::CommandsMapIterator(CommandsMap &m)
{
    p = new CommandsMapIteratorPrivate(*m.p);
}

CommandsMapIterator::~CommandsMapIterator()
{
    delete p;
}

CommandDef *CommandsMapIterator::operator++()
{
    if (p->it == p->map.end())
        return NULL;
    CommandDef *res = &(*p->it).second;
    ++(p->it);
    return res;
}

}


