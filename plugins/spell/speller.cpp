/***************************************************************************
                          speller.cpp  -  description
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

#include "speller.h"

#include "log.h"

#include <QFile>
#include <QStringList>

#ifdef Q_OS_WIN

#include <QLibrary>

SpellerBase::SpellerBase(const QString &path)
{
    init();
    QString p = path;
    int n = p.lastIndexOf('\\');
    if (n >= 0)
        p = p.left(n);
    p += "\\aspell-15.dll";
    m_aspellLib = new QLibrary(p);

    if (m_aspellLib->load() && m_aspellLib->isLoaded()){
        (void*&)_new_aspell_config                      = m_aspellLib->resolve("new_aspell_config");
        (void*&)_delete_aspell_config                   = m_aspellLib->resolve("new_aspell_config");
        (void*&)_get_aspell_dict_info_list              = m_aspellLib->resolve("get_aspell_dict_info_list");
        (void*&)_aspell_dict_info_list_elements         = m_aspellLib->resolve("aspell_dict_info_list_elements");
        (void*&)_delete_aspell_dict_info_enumeration    = m_aspellLib->resolve("delete_aspell_dict_info_enumeration");
        (void*&)_aspell_dict_info_enumeration_next      = m_aspellLib->resolve("aspell_dict_info_enumeration_next");
        (void*&)_aspell_config_replace                  = m_aspellLib->resolve("aspell_config_replace");
        (void*&)_new_aspell_speller                     = m_aspellLib->resolve("new_aspell_speller");
        (void*&)_to_aspell_speller                      = m_aspellLib->resolve("to_aspell_speller");
        (void*&)_delete_aspell_speller                  = m_aspellLib->resolve("delete_aspell_speller");
        (void*&)_aspell_error_message                   = m_aspellLib->resolve("aspell_error_message");
        (void*&)_aspell_error                           = m_aspellLib->resolve("aspell_error");
        (void*&)_delete_aspell_can_have_error           = m_aspellLib->resolve("delete_aspell_can_have_error");
        (void*&)_aspell_speller_check                   = m_aspellLib->resolve("aspell_speller_check");
        (void*&)_aspell_speller_suggest                 = m_aspellLib->resolve("aspell_speller_suggest");
        (void*&)_aspell_word_list_elements              = m_aspellLib->resolve("aspell_word_list_elements");
        (void*&)_aspell_string_enumeration_next         = m_aspellLib->resolve("aspell_string_enumeration_next");
        (void*&)_aspell_speller_add_to_personal         = m_aspellLib->resolve("aspell_speller_add_to_personal");
        if ((_new_aspell_config == NULL) ||
                (_delete_aspell_config == NULL) ||
                (_get_aspell_dict_info_list == NULL) ||
                (_aspell_dict_info_list_elements == NULL) ||
                (_delete_aspell_dict_info_enumeration == NULL) ||
                (_aspell_dict_info_enumeration_next == NULL) ||
                (_aspell_config_replace == NULL) ||
                (_new_aspell_speller == NULL) ||
                (_to_aspell_speller == NULL) ||
                (_delete_aspell_speller == NULL) ||
                (_aspell_error_message == NULL) ||
                (_aspell_error == NULL) ||
                (_delete_aspell_can_have_error == NULL) ||
                (_aspell_speller_check == NULL) ||
                (_aspell_speller_suggest == NULL) ||
                (_aspell_word_list_elements == NULL) ||
                (_aspell_string_enumeration_next == NULL) ||
                (_aspell_speller_add_to_personal == NULL)){
            delete m_aspellLib;
            m_aspellLib = NULL;
            init();
        }
    }
}

SpellerBase::~SpellerBase()
{
    delete m_aspellLib;
}

void SpellerBase::init()
{
    _new_aspell_config = NULL;
    _delete_aspell_config = NULL;
    _get_aspell_dict_info_list = NULL;
    _aspell_dict_info_list_elements = NULL;
    _delete_aspell_dict_info_enumeration = NULL;
    _aspell_dict_info_enumeration_next = NULL;
    _aspell_config_replace = NULL;
    _new_aspell_speller = NULL;
    _to_aspell_speller = NULL;
    _delete_aspell_speller = NULL;
    _aspell_error_message = NULL;
    _aspell_error = NULL;
    _delete_aspell_can_have_error = NULL;
    _aspell_speller_check = NULL;
    _aspell_speller_suggest = NULL;
    _aspell_word_list_elements = NULL;
    _aspell_string_enumeration_next = NULL;
    _aspell_speller_add_to_personal = NULL;
}

#define new_aspell_config()                         m_base._new_aspell_config()
#define delete_aspell_config(cfg)                   m_base._delete_aspell_config(cfg)
#define get_aspell_dict_info_list(cfg)              m_base._get_aspell_dict_info_list(cfg)
#define aspell_dict_info_list_elements(dlist)       m_base._aspell_dict_info_list_elements(dlist)
#define delete_aspell_dict_info_enumeration(dlist)  m_base._delete_aspell_dict_info_enumeration(dlist)
#define aspell_dict_info_enumeration_next(dlist)    m_base._aspell_dict_info_enumeration_next(dlist)
#define aspell_config_replace(c, k, v)              m_base._aspell_config_replace(c, k, v)
#define new_aspell_speller(dlist)                   m_base._new_aspell_speller(dlist)
#define to_aspell_speller(dlist)                    m_base._to_aspell_speller(dlist)
#define delete_aspell_speller(dlist)                m_base._delete_aspell_speller(dlist)
#define aspell_error_message(dlist)                 m_base._aspell_error_message(dlist)
#define aspell_error(dlist)                         m_base._aspell_error(dlist)
#define delete_aspell_can_have_error(dlist)         m_base._delete_aspell_can_have_error(dlist)
#define aspell_speller_check(c, v, s)               m_base._aspell_speller_check(c, v, s)
#define aspell_speller_suggest(c, v, s)             m_base._aspell_speller_suggest(c, v, s)
#define aspell_word_list_elements(c)                m_base._aspell_word_list_elements(c)
#define aspell_string_enumeration_next(c)           m_base._aspell_string_enumeration_next(c)
#define aspell_speller_add_to_personal(c, v, s)     m_base._aspell_speller_suggest(c, v, s)

#else

SpellerBase::SpellerBase(const QString &)
{}

SpellerBase::~SpellerBase()
{}

#endif


SpellerConfig::SpellerConfig(SpellerBase &base)
    : m_base(base)
{
#ifdef WIN32
    cfg = NULL;
    if (m_base._new_aspell_config)
        cfg = new_aspell_config();
#else
    cfg = new_aspell_config();
#endif
}

SpellerConfig::~SpellerConfig()
{
    if (cfg)
        delete_aspell_config(cfg);
}

QString SpellerConfig::getLangs()
{
    QString res;
    if (cfg == NULL)
        return res;
    AspellDictInfoList *dlist = get_aspell_dict_info_list(cfg);
    AspellDictInfoEnumeration *dels = aspell_dict_info_list_elements(dlist);
    const AspellDictInfo *entry;
    while ((entry = aspell_dict_info_enumeration_next(dels)) != NULL){
        if (!res.isEmpty())
            res += ';';
        res += entry->name;
    }
    delete_aspell_dict_info_enumeration(dels);
    return res;
}

int SpellerConfig::setKey(const char *key, const QString &val)
{
    if (cfg == NULL)
        return -1;
    return aspell_config_replace(cfg, key, val.toUtf8());
}

Speller::Speller(SpellerConfig *cfg)
        : m_base(cfg->m_base)
{
    speller = NULL;
    if (cfg->cfg){
        AspellCanHaveError *ret = new_aspell_speller(cfg->cfg);
        if (aspell_error(ret) != 0){
            SIM::log(SIM::L_WARN, "Spell: %s", aspell_error_message(ret));
            delete_aspell_can_have_error(ret);
            return;
        }
        speller = to_aspell_speller(ret);
    }
}

Speller::~Speller()
{
    if (speller)
        delete_aspell_speller(speller);
}

int Speller::check(const char *word)
{
    if (speller == NULL)
        return -1;
    return aspell_speller_check(speller, word, strlen(word));
}

bool Speller::add(const char *word)
{
    if (speller == NULL)
        return false;
    return aspell_speller_check(speller, word, strlen(word)) != 0;
}

QStringList Speller::suggestions(const char *word)
{
    QStringList res;
    const AspellWordList *wl = aspell_speller_suggest(speller, word, -1);
    if (wl){
        AspellStringEnumeration *els = aspell_word_list_elements(wl);
        const char *word;
        while ((word = aspell_string_enumeration_next(els)) != NULL) {
            res.append(QString::fromUtf8(word));
        }
    }
    return res;
}
