/***************************************************************************
                          osdiface.cpp  -  description
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

#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>

#include "simgui/fontedit.h"
#include "misc.h"
#include "simgui/qcolorbutton.h"

#include "osdiface.h"
#include "osd.h"

using namespace SIM;

OSDIface::OSDIface(QWidget *parent, SIM::PropertyHubPtr data, OSDPlugin *plugin) : QWidget(parent)
{
    setupUi(this);
    m_plugin = plugin;
#ifndef WIN32
    chkFading->setChecked(false);
    chkFading->hide();
#endif
    cmbPos->addItem(i18n("Left-bottom"));
    cmbPos->addItem(i18n("Left-top"));
    cmbPos->addItem(i18n("Right-bottom"));
    cmbPos->addItem(i18n("Right-top"));
    cmbPos->addItem(i18n("Center-bottom"));
    cmbPos->addItem(i18n("Center-top"));
    cmbPos->addItem(i18n("Center"));
    cmbPos->setCurrentIndex(data->value("Position").toUInt());
    spnOffs->setMinimum(0);
    spnOffs->setMaximum(500);
    spnOffs->setValue(data->value("Offset").toUInt());
    spnTimeout->setMinimum(1);
    spnTimeout->setMaximum(60);
    spnTimeout->setValue(data->value("Timeout").toUInt());
    btnColor->setColor(data->value("Color").toUInt());
    if (data->value("Font").toString().isEmpty()){
        edtFont->setFont(FontEdit::font2str(plugin->getBaseFont(font()), false));
    }else{
        edtFont->setFont(data->value("Font").toString());
    }
    chkShadow->setChecked(data->value("Shadow").toBool());
    chkFading->setChecked(data->value("Fading").toBool());
    if (data->value("Background").toBool()){
        chkBackground->setChecked(true);
        btnBgColor->setColor(data->value("BgColor").toUInt());
    }else{
        chkBackground->setChecked(false);
    }
    bgToggled(data->value("Background").toBool());
    connect(chkBackground, SIGNAL(toggled(bool)), this, SLOT(bgToggled(bool)));
    unsigned nScreens = screens();
    if (nScreens <= 1){
        lblScreen->hide();
        cmbScreen->hide();
    }else{
        for (unsigned i = 0; i < nScreens; i++)
            cmbScreen->addItem(QString::number(i));
        unsigned curScreen = data->value("Screen").toUInt();
        if (curScreen >= nScreens)
            curScreen = 0;
        cmbScreen->setCurrentIndex(curScreen);
    }
}

void OSDIface::bgToggled(bool bState)
{
    if (bState){
        btnBgColor->setEnabled(true);
        return;
    }
    btnBgColor->setColor(palette().color(QPalette::Base));
    btnBgColor->setEnabled(false);
}

void OSDIface::apply(SIM::PropertyHubPtr data)
{
    data->setValue("Position", cmbPos->currentIndex());
    data->setValue("Offset", spnOffs->text().toUInt());
    data->setValue("Timeout", spnTimeout->text().toUInt());
    data->setValue("Color", btnColor->color().rgb());
    QString f = edtFont->getFont();
    QString base = FontEdit::font2str(m_plugin->getBaseFont(font()), false);
    if (f == base)
        f.clear();
    data->setValue("Font", f);
    data->setValue("Shadow", chkShadow->isChecked());
    data->setValue("Fading", chkFading->isChecked());
    data->setValue("Background", chkBackground->isChecked());
    if (data->value("Background").toBool()){
        data->setValue("BgColor", btnBgColor->color().rgb());
    }else{
        data->setValue("BgColor", 0);
    }
    unsigned nScreens = screens();
    if (nScreens <= 1){
        data->setValue("Screen", 0);
    }else{
        data->setValue("Screen", cmbScreen->currentIndex());
    }
}

