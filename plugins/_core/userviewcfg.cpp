/***************************************************************************
                          userviewcfg.cpp  -  description
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

#include "icons.h"
#include "userviewcfg.h"
#include "core.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>

using namespace SIM;

UserViewConfig::UserViewConfig(QWidget *parent) : QWidget(parent)
{
    setupUi(this);
    chkDblClick->setChecked(CorePlugin::instance()->value("UseDblClick").toBool());
    chkSysColors->setChecked(CorePlugin::instance()->value("UseSysColors").toBool());
    btnOnline->setColor(CorePlugin::instance()->value("ColorOnline").toUInt());
    btnOffline->setColor(CorePlugin::instance()->value("ColorOffline").toUInt());
    btnAway->setColor(CorePlugin::instance()->value("ColorAway").toUInt());
    btnNA->setColor(CorePlugin::instance()->value("ColorNA").toUInt());
    btnDND->setColor(CorePlugin::instance()->value("ColorDND").toUInt());
    btnGroup->setColor(CorePlugin::instance()->value("ColorGroup").toUInt());
    chkGroupSeparator->setChecked(CorePlugin::instance()->value("GroupSeparator").toBool());
    chkSmallFont->setChecked(CorePlugin::instance()->value("SmallGroupFont").toBool());
    chkScroll->setChecked(CorePlugin::instance()->value("NoScroller").toBool());
    connect(chkSysColors, SIGNAL(toggled(bool)), this, SLOT(colorsToggled(bool)));
    colorsToggled(chkSysColors->isChecked());
    fillBox(cmbSort1);
    fillBox(cmbSort2);
    fillBox(cmbSort3);
	CorePlugin::instance()->setValue("SortMode", CorePlugin::instance()->value("SortMode").toUInt());
    connect(cmbSort1, SIGNAL(activated(int)), this, SLOT(sortChanged(int)));
    connect(cmbSort2, SIGNAL(activated(int)), this, SLOT(sortChanged(int)));
    connect(cmbSort3, SIGNAL(activated(int)), this, SLOT(sortChanged(int)));
    btnAuth1->setIcon(Icon("text_strike"));
    btnAuth2->setIcon(Icon("text_italic"));
    btnAuth3->setIcon(Icon("text_under"));
    btnVisible1->setIcon(Icon("text_strike"));
    btnVisible2->setIcon(Icon("text_italic"));
    btnVisible3->setIcon(Icon("text_under"));
    btnInvisible1->setIcon(Icon("text_strike"));
    btnInvisible2->setIcon(Icon("text_italic"));
    btnInvisible3->setIcon(Icon("text_under"));
    btnAuth1->setChecked(true);
    btnAuth2->setChecked(true);
    btnAuth3->setChecked(true);
    btnVisible1->setChecked(true);
    btnVisible2->setChecked(true);
    btnVisible3->setChecked(true);
    btnInvisible1->setChecked(true);
    btnInvisible2->setChecked(true);
    btnInvisible3->setChecked(true);
    connect(btnAuth1, SIGNAL(toggled(bool)), this, SLOT(setFonts(bool)));
    connect(btnAuth2, SIGNAL(toggled(bool)), this, SLOT(setFonts(bool)));
    connect(btnAuth3, SIGNAL(toggled(bool)), this, SLOT(setFonts(bool)));
    connect(btnVisible1, SIGNAL(toggled(bool)), this, SLOT(setFonts(bool)));
    connect(btnVisible2, SIGNAL(toggled(bool)), this, SLOT(setFonts(bool)));
    connect(btnVisible3, SIGNAL(toggled(bool)), this, SLOT(setFonts(bool)));
    connect(btnInvisible1, SIGNAL(toggled(bool)), this, SLOT(setFonts(bool)));
    connect(btnInvisible2, SIGNAL(toggled(bool)), this, SLOT(setFonts(bool)));
    connect(btnInvisible3, SIGNAL(toggled(bool)), this, SLOT(setFonts(bool)));
    btnAuth1->setChecked((CorePlugin::instance()->value("AuthStyle").toUInt() & STYLE_STRIKE) != 0);
    btnAuth2->setChecked((CorePlugin::instance()->value("AuthStyle").toUInt() & STYLE_ITALIC) != 0);
    btnAuth3->setChecked((CorePlugin::instance()->value("AuthStyle").toUInt() & STYLE_UNDER) != 0);
    btnVisible1->setChecked((CorePlugin::instance()->value("VisibleStyle").toUInt() & STYLE_STRIKE) != 0);
    btnVisible2->setChecked((CorePlugin::instance()->value("VisibleStyle").toUInt() & STYLE_ITALIC) != 0);
    btnVisible3->setChecked((CorePlugin::instance()->value("VisibleStyle").toUInt() & STYLE_UNDER) != 0);
    btnInvisible1->setChecked((CorePlugin::instance()->value("InvisibleStyle").toUInt() & STYLE_STRIKE) != 0);
    btnInvisible2->setChecked((CorePlugin::instance()->value("InvisibleStyle").toUInt() & STYLE_ITALIC) != 0);
    btnInvisible3->setChecked((CorePlugin::instance()->value("InvisibleStyle").toUInt() & STYLE_UNDER) != 0);
    setFonts(true);
}

UserViewConfig::~UserViewConfig()
{
}

void UserViewConfig::apply()
{
    CorePlugin::instance()->setValue("UseDblClick", chkDblClick->isChecked());
    CorePlugin::instance()->setValue("UseSysColors", chkSysColors->isChecked());
    CorePlugin::instance()->setValue("GroupSeparator", chkGroupSeparator->isChecked());
    CorePlugin::instance()->setValue("SortMode", getSortMode());
    CorePlugin::instance()->setValue("SmallGroupFont", chkSmallFont->isChecked());
    CorePlugin::instance()->setValue("NoScroller", chkScroll->isChecked());
    if (CorePlugin::instance()->value("UseSysColors").toBool())
    {
        CorePlugin::instance()->setValue("ColorOnline", 0);
        CorePlugin::instance()->setValue("ColorOffline", 0);
        CorePlugin::instance()->setValue("ColorAway", 0);
        CorePlugin::instance()->setValue("ColorNA", 0);
        CorePlugin::instance()->setValue("ColorDND", 0);
        CorePlugin::instance()->setValue("ColorGroup", 0);
    }
    else
    {
        CorePlugin::instance()->setValue("ColorOnline", btnOnline->color().rgb());
        CorePlugin::instance()->setValue("ColorOffline", btnOffline->color().rgb());
        CorePlugin::instance()->setValue("ColorAway", btnAway->color().rgb());
        CorePlugin::instance()->setValue("ColorNA", btnNA->color().rgb());
        CorePlugin::instance()->setValue("ColorDND", btnDND->color().rgb());
        CorePlugin::instance()->setValue("ColorGroup", btnGroup->color().rgb());
    }
    unsigned style = 0;
    if (btnAuth1->isChecked()) style |= STYLE_STRIKE;
    if (btnAuth2->isChecked()) style |= STYLE_ITALIC;
    if (btnAuth3->isChecked()) style |= STYLE_UNDER;
    CorePlugin::instance()->setValue("AuthStyle", style);
    style = 0;
    if (btnVisible1->isChecked()) style |= STYLE_STRIKE;
    if (btnVisible2->isChecked()) style |= STYLE_ITALIC;
    if (btnVisible3->isChecked()) style |= STYLE_UNDER;
    CorePlugin::instance()->setValue("VisibleStyle", style);
    style = 0;
    if (btnInvisible1->isChecked()) style |= STYLE_STRIKE;
    if (btnInvisible2->isChecked()) style |= STYLE_ITALIC;
    if (btnInvisible3->isChecked()) style |= STYLE_UNDER;
    CorePlugin::instance()->setValue("InvisibleStyle", style);
    EventRepaintView e;
    e.process();
}

void UserViewConfig::colorsToggled(bool state)
{
    if (state)
    {
        QColor textColor = palette().color(QPalette::Active, QPalette::Text);
        QColor disabledColor = palette().color(QPalette::Disabled, QPalette::Text);
        btnOnline->setColor(textColor);
        btnOffline->setColor(disabledColor);
        btnAway->setColor(disabledColor);
        btnNA->setColor(disabledColor);
        btnDND->setColor(disabledColor);
        btnGroup->setColor(disabledColor);
    }
    btnOnline->setEnabled(!state);
    btnOffline->setEnabled(!state);
    btnAway->setEnabled(!state);
    btnNA->setEnabled(!state);
    btnDND->setEnabled(!state);
    btnGroup->setEnabled(!state);
    lblOnline->setEnabled(!state);
    lblOffline->setEnabled(!state);
    lblAway->setEnabled(!state);
    lblNA->setEnabled(!state);
    lblDND->setEnabled(!state);
    lblGroup->setEnabled(!state);
    lblColors->setEnabled(!state);
}

void UserViewConfig::fillBox(QComboBox *cmb)
{
    cmb->insertItem(INT_MAX,i18n("Status"));
    cmb->insertItem(INT_MAX,i18n("Last message time"));
    cmb->insertItem(INT_MAX,i18n("Contact name"));
    cmb->insertItem(INT_MAX,"");
}

void UserViewConfig::setSortMode(unsigned mode)
{
    QComboBox *cmb[3] = { cmbSort1, cmbSort2, cmbSort3 };
    unsigned i;
    for (i = 0; i < 3; i++)
    {
        cmb[i]->setEnabled(true);
        unsigned m = mode & 0xFF;
        mode = mode >> 8;
        if (m == 0)
        {
            cmb[i++]->setCurrentIndex(3);
            break;
        }
        cmb[i]->setCurrentIndex(m - 1);
    }
    for (;i < 3; i++)
    {
        cmb[i]->setCurrentIndex(3);
        cmb[i]->setEnabled(false);
    }
}

void UserViewConfig::sortChanged(int)
{
    setSortMode(getSortMode());
}

void UserViewConfig::setFonts(bool)
{
    QFont fAuth(font());
    fAuth.setStrikeOut(btnAuth1->isChecked());
    fAuth.setItalic(btnAuth2->isChecked());
    fAuth.setUnderline(btnAuth3->isChecked());
    lblAuth->setFont(fAuth);
    QFont fVisible(font());
    fVisible.setStrikeOut(btnVisible1->isChecked());
    fVisible.setItalic(btnVisible2->isChecked());
    fVisible.setUnderline(btnVisible3->isChecked());
    lblVisible->setFont(fVisible);
    QFont fInvisible(font());
    fInvisible.setStrikeOut(btnInvisible1->isChecked());
    fInvisible.setItalic(btnInvisible2->isChecked());
    fInvisible.setUnderline(btnInvisible3->isChecked());
    lblInvisible->setFont(fInvisible);
}

unsigned UserViewConfig::getSortMode()
{
    unsigned m1 = cmbSort1->currentIndex() + 1;
    if (m1 > 3)
        m1 = 0;
    unsigned m2 = cmbSort2->currentIndex() + 1;
    if (m2 > 3)
        m2 = 0;
    unsigned m3 = cmbSort3->currentIndex() + 1;
    if (m3 > 3)
        m3 = 0;
    if (m1)
    {
        if (m2 == m1)
            m2 = 0;
        if (!m2 || m3 == m1 || m3 == m2)
            m3 = 0;
    }
    else
    {
        m2 = 0;
        m3 = 0;
    }
    return (m3 << 16) + (m2 << 8) + m1;
}
