/***************************************************************************
                          cmddef.h  -  description
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

#ifndef _CMDDEF_H
#define _CMDDEF_H

#include "simapi.h"

class QString;
class QVariant;

namespace SIM {

struct CommandDef;
class CommandsDef;

class EXPORT CommandsList
{
public:
    CommandsList(CommandsDef &def, bool bFull = false);
    ~CommandsList();
    CommandDef *operator++();
    void reset();
private:
    class CommandsListPrivate *p;
    friend class CommandsListPrivate;

    COPY_RESTRICTED(CommandsList)
};

class EXPORT CommandsDef
{
public:
    CommandsDef(unsigned id, bool bMenu);
    ~CommandsDef();
    unsigned id();
    bool isMenu();
    void setConfig(const QString &cfg_str);
    void setConfig(const QVariant &cfg_variant);
    void set(const CommandDef *def);
    void set(const CommandDef &def);
private:
    class CommandsDefPrivate *p;
    friend class CommandsList;
    friend class CommandsDefPrivate;

    COPY_RESTRICTED(CommandsDef)
};

class EXPORT CommandsMap
{
public:
    CommandsMap();
    ~CommandsMap();
    CommandDef *find(unsigned id);
    bool add(CommandDef*);
    bool erase(unsigned id);
    void clear();
private:
    class CommandsMapPrivate  *p;
    friend class CommandsMapIterator;

    COPY_RESTRICTED(CommandsMap)
};

class EXPORT CommandsMapIterator
{
public:
    CommandsMapIterator(CommandsMap&);
    ~CommandsMapIterator();
    CommandDef *operator++();
private:
    class CommandsMapIteratorPrivate *p;

    COPY_RESTRICTED(CommandsMapIterator)
};


} // namespace SIM

#endif

// vim: set expandtab:
