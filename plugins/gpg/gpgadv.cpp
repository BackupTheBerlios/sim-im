/***************************************************************************
                          gpgadv.cpp  -  description
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

#include "gpgadv.h"
#include "gpg.h"

#include <QLineEdit>

GpgAdvanced::GpgAdvanced(QWidget *parent, GpgPlugin *plugin) : QWidget(parent)
{
	setupUi(this);
    m_plugin = plugin;
    edtGenKey->setText(m_plugin->value("GenKey").toString());
    edtPublic->setText(m_plugin->value("PublicList").toString());
    edtSecret->setText(m_plugin->value("SecretList").toString());
    edtExport->setText(m_plugin->value("Export").toString());
    edtImport->setText(m_plugin->value("Import").toString());
    edtEncrypt->setText(m_plugin->value("Encrypt").toString());
    edtDecrypt->setText(m_plugin->value("Decrypt").toString());
}

GpgAdvanced::~GpgAdvanced()
{
}

void GpgAdvanced::apply()
{
    m_plugin->setValue("GenKey", edtGenKey->text());
    m_plugin->setValue("PublicList", edtPublic->text());
    m_plugin->setValue("SecretList", edtSecret->text());
    m_plugin->setValue("Export", edtExport->text());
    m_plugin->setValue("Import", edtImport->text());
    m_plugin->setValue("Encrypt", edtEncrypt->text());
    m_plugin->setValue("Decrypt", edtDecrypt->text());
}


