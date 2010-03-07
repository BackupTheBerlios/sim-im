/***************************************************************************
                          speller.h  -  description
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

#ifndef _SPELLER_H
#define _SPELLER_H

#include "simapi.h"

#include <aspell.h>
#include <QString>
#include <QStringList>

class SpellerBase
{
public:
    SpellerBase(const QString &path);
    ~SpellerBase();

#ifdef WIN32
    struct AspellConfig *(*_new_aspell_config)();
    void (*_delete_aspell_config)(struct AspellConfig * ths);
    struct AspellDictInfoList *(*_get_aspell_dict_info_list)(struct AspellConfig * config);
    struct AspellDictInfoEnumeration *(*_aspell_dict_info_list_elements)(const struct AspellDictInfoList * ths);
    void (*_delete_aspell_dict_info_enumeration)(struct AspellDictInfoEnumeration * ths);
    const struct AspellDictInfo *(*_aspell_dict_info_enumeration_next)(struct AspellDictInfoEnumeration * ths);
    int (*_aspell_config_replace)(struct AspellConfig * ths, const char * key, const char * value);
    struct AspellCanHaveError *(*_new_aspell_speller)(struct AspellConfig * config);
    struct AspellSpeller *(*_to_aspell_speller)(struct AspellCanHaveError * obj);
    void (*_delete_aspell_speller)(struct AspellSpeller * ths);
    const char *(*_aspell_error_message)(const struct AspellCanHaveError * ths);
    const struct AspellError *(*_aspell_error)(const struct AspellCanHaveError * ths);
    void (*_delete_aspell_can_have_error)(struct AspellCanHaveError * ths);
    int (*_aspell_speller_check)(struct AspellSpeller * ths, const char * word, int word_size);
    const struct AspellWordList *(*_aspell_speller_suggest)(struct AspellSpeller * ths, const char * word, int word_size);
    struct AspellStringEnumeration *(*_aspell_word_list_elements)(const struct AspellWordList * ths);
    const char *(*_aspell_string_enumeration_next)(struct AspellStringEnumeration * ths);
    int (*_aspell_speller_add_to_personal)(struct AspellSpeller * ths, const char * word, int word_size);
protected:
    void init();
    class QLibrary *m_aspellLib;
#endif
};

class SpellerConfig
{
    COPY_RESTRICTED(SpellerConfig);
public:
    SpellerConfig(SpellerBase &base);
    ~SpellerConfig();
    QString getLangs();
    int setKey(const char *key, const QString &val);
protected:
    struct AspellConfig *cfg;
    SpellerBase &m_base;
    friend class Speller;
};

class Speller
{
    COPY_RESTRICTED(Speller);
public:
    Speller(SpellerConfig *cfg);
    ~Speller();
    bool created() { return speller != NULL; }
    int check(const char *word);
    QStringList suggestions(const char *word);
    bool add(const char *word);
protected:
    struct AspellSpeller *speller;
    SpellerBase &m_base;
};

#endif

