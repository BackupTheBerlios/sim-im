/***************************************************************************
                          spellcfg.cpp  -  description
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

#include "spellcfg.h"
#include "spell.h"
#include "speller.h"
#ifdef WIN32
#include "spellfind.h"
#endif

#include "log.h"
#include "simgui/editfile.h"

using namespace SIM;

SpellConfig::SpellConfig(QWidget *parent, SpellPlugin *plugin) : QWidget(parent)
{
    setupUi(this);
    m_plugin = plugin;
#ifdef WIN32
    edtPath->setText(m_plugin->value("Path").toString());
    edtPath->setFilter(i18n("ASpell(aspell.exe)"));
    m_find = NULL;
#else
    lblPath->hide();
    edtPath->hide();
#endif
    connect(edtPath, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(btnFind, SIGNAL(clicked()), this, SLOT(find()));
    textChanged(edtPath->text());
}

SpellConfig::~SpellConfig()
{
#ifdef WIN32
    delete m_find;
#endif
}

void SpellConfig::apply()
{
#ifdef WIN32
    m_plugin->setValue("Path", edtPath->text());
#endif
    QString lang;
    for(int c = 0; c < lstLang->count(); c++)
    {
        QListWidgetItem *item = lstLang->item( c );
        if (item->checkState() == Qt::Unchecked)
            continue;
        if (!lang.isEmpty())
            lang += ';';
        lang += item->text();
    }
    m_plugin->setValue("Lang", lang);
    m_plugin->reset();
}

void SpellConfig::textChanged(const QString &str)
{
    QString langs;
#ifdef WIN32
    if (str.isEmpty()){
        lnkAspell->show();
        btnFind->show();
    }else{
#endif
        lnkAspell->hide();
        btnFind->hide();
        SpellerBase base(str);
        SpellerConfig cfg(base);
        langs = cfg.getLangs();
#ifdef WIN32
    }
#endif
    lstLang->clear();
    if (langs.isEmpty()){
        lblLang->setEnabled(false);
        lstLang->setEnabled(false);
    }else{
        lblLang->setEnabled(true);
        lstLang->setEnabled(true);
        int r = 0;
        while (!langs.isEmpty()){
            QString l = SIM::getToken(langs, ';');
            bool bCheck = false;
            QString ll = m_plugin->value("Lang").toString();
            while (!ll.isEmpty()){
                QString lc = SIM::getToken(ll, ';');
                if (l == lc){
                    bCheck = true;
                    break;
                }
            }
            QListWidgetItem* item = new QListWidgetItem();
            item->setText(l);
            item->setFlags( item->flags() | Qt::ItemIsUserCheckable );
            item->setCheckState( bCheck ? Qt::Checked : Qt::Unchecked );
            lstLang->insertItem( r, item );
            r++;
        }
    }
}

void SpellConfig::find()
{
#ifdef WIN32
    if (m_find == NULL){
        m_find = new SpellFind(edtPath);
        connect(m_find, SIGNAL(finished()), this, SLOT(findFinished()));
    }
    raiseWindow(m_find);
#endif
}

void SpellConfig::findFinished()
{
#ifdef WIN32
    m_find = NULL;
#endif
}

// vim: set expandtab: 
