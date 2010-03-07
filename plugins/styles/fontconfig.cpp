/***************************************************************************
                          fontconfig.cpp  -  description
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

#include "fontconfig.h"
#include "simgui/fontedit.h"
#include "styles.h"
#include "simgui/qcolorbutton.h"

#include <QCheckBox>
#include <QApplication>
#include <QMenu>

FontConfig::FontConfig(QWidget *parent, StylesPlugin *plugin)
        : QWidget(parent)
{
    setupUi(this);
    m_plugin = plugin;
    connect(chkSystem, SIGNAL(toggled(bool)), this, SLOT(systemToggled(bool)));
    connect(chkColors, SIGNAL(toggled(bool)), this, SLOT(colorsToggled(bool)));
    chkSystem->setChecked(m_plugin->value("SystemFonts").toBool());
    systemToggled(chkSystem->isChecked());
    if (!chkSystem->isChecked()){
        QMenu m;
        QFont base = QApplication::font();
        QFont menu = QApplication::font(&m);
        base = FontEdit::str2font(m_plugin->value("BaseFont").toString(), base);
        menu = FontEdit::str2font(m_plugin->value("MenuFont").toString(), menu);
        edtFont->setFont(FontEdit::font2str(base, true));
        edtMenu->setFont(FontEdit::font2str(menu, true));
    }

    chkColors->setChecked(m_plugin->value("SystemColors").toBool());
    colorsToggled(chkColors->isChecked());
}

FontConfig::~FontConfig()
{
}

void FontConfig::apply()
{
    QString base;
    QString menu;
    if (chkSystem->isChecked()){
        m_plugin->setValue("SystemFonts", true);
    }else{
        m_plugin->setValue("SystemFonts", false);
        base = edtFont->getFont();
        menu = edtMenu->getFont();
    }
    m_plugin->setValue("BaseFont", base);
    m_plugin->setValue("MenuFont", menu);
    m_plugin->setFonts();

    bool bChanged = false;
    if (chkColors->isChecked()){
        if (!m_plugin->value("SystemColors").toBool()){
            m_plugin->setValue("SystemColors", true);
            bChanged = true;
        }
    }else{
        if (m_plugin->value("SystemColors").toBool()){
            bChanged = true;
        }else{
            bChanged = ((btnBtnColor->color().rgb() & 0xFFFFFF) != m_plugin->value("BtnColor").toUInt()) ||
                       ((btnBgColor->color().rgb() & 0xFFFFFF) != m_plugin->value("BgColor").toUInt());
        }
        m_plugin->setValue("SystemColors", false);
        if (bChanged){
            m_plugin->setValue("BtnColor", btnBtnColor->color().rgb() & 0xFFFFFF);
            m_plugin->setValue("BgColor", btnBgColor->color().rgb() & 0xFFFFFF);
        }
    }
    if (bChanged)
        m_plugin->setColors();
}

void FontConfig::systemToggled(bool bState)
{
    edtFont->setEnabled(!bState);
    edtMenu->setEnabled(!bState);
    if (bState){
        m_plugin->setupDefaultFonts();
        edtFont->setFont(FontEdit::font2str(*m_plugin->m_saveBaseFont, true));
        edtMenu->setFont(FontEdit::font2str(*m_plugin->m_saveMenuFont, true));
    }
}

void FontConfig::colorsToggled(bool bState)
{
    btnBtnColor->setEnabled(!bState);
    btnBgColor->setEnabled(!bState);
    if (!bState){
        btnBtnColor->setColor(QColor(m_plugin->value("BtnColor").toUInt() & 0xFFFFFF));
        btnBgColor->setColor(QColor(m_plugin->value("BgColor").toUInt() & 0xFFFFFF));
    }
}

