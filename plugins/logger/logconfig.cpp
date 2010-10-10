/***************************************************************************
                          logconfig.cpp  -  description
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

#include <QStyle>
#include <QLayout>
#include <QPixmap>
#include <QBitmap>
#include <QAbstractButton>
#include <QFileInfo>
#include <QStyleOption>

#include "simgui/editfile.h"
#include "simgui/listview.h"
#include "log.h"
#include "misc.h"

#include "logconfig.h"
#include "logger.h"

using namespace SIM;

const unsigned COL_NAME		= 0;
const unsigned COL_CHECK	= 1;
const unsigned COL_CHECKED	= 2;
const unsigned COL_LEVEL	= 3;
const unsigned COL_PACKET	= 4;

LogConfig::LogConfig(QWidget *parent, LoggerPlugin *plugin)
  : QWidget(parent)
  , m_plugin(plugin)
{
    setupUi(this);

    edtFile->setText(m_plugin->value("File").toString());
    edtFile->setCreate(true);
    fill();
}

void LogConfig::apply()
{
    unsigned log_level = 0;
    /* test if file exist */
//    if(!edtFile->text().isEmpty()) {
//      QFile file(edtFile->text());
//      if (!file.open(QIODevice::Append | QIODevice::ReadWrite)) {
//          log(L_DEBUG,"Logfile %s isn't a valid file - discarded!", qPrintable(edtFile->text()));
//          edtFile->setText(QString());
//      } else {
//          file.close();
//      }
//      m_plugin->setValue("File", edtFile->text());
//    }

//    /* check selected protocols */
//    for (int row = 0; row < lstLevel->count(); ++row) {
//        QListWidgetItem *item = lstLevel->item(row);
//        unsigned level  = item->data(Qt::UserRole).toUInt();
//        unsigned packet =  item->data(Qt::UserRole).toUInt();
//        if (item->checkState() == Qt::Checked){
//            if (level){
//                log_level |= level;
//            }else{
//                m_plugin->setLogType(packet, true);
//            }
//        }else{
//            if (level == 0)
//                m_plugin->setLogType(packet, false);
//        }
//    }
    m_plugin->setValue("LogLevel", log_level);
    m_plugin->openFile();
}

static QListWidgetItem *createItem(const QString &text, bool bChecked, unsigned id, bool bPacket = false)
{
    QListWidgetItem *item = new QListWidgetItem(text);
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
    item->setCheckState(bChecked ? Qt::Checked : Qt::Unchecked);
    item->setData(bPacket ? Qt::UserRole + 1 : Qt::UserRole, id);
    return item;
}

void LogConfig::fill()
{
    lstLevel->clear();

    lstLevel->addItem(createItem(i18n("Error"),   (m_plugin->value("LogLevel").toUInt() & L_ERROR  ) != 0, L_ERROR));
    lstLevel->addItem(createItem(i18n("Warning"), (m_plugin->value("LogLevel").toUInt() & L_WARN   ) != 0, L_WARN));
    lstLevel->addItem(createItem(i18n("Debug"),   (m_plugin->value("LogLevel").toUInt() & L_DEBUG  ) != 0, L_DEBUG));
    lstLevel->addItem(createItem(i18n("Packets"), (m_plugin->value("LogLevel").toUInt() & L_PACKETS) != 0, L_PACKETS));
    //lstLevel->addItem(createItem(i18n("Events"),  (m_plugin->getLogLevel() & L_EVENTS ) != 0, L_EVENTS);

//    PacketType *type;
//    ContactList::PacketIterator it;
//    while ((type = ++it) != NULL){
//       lstLevel->addItem(createItem(type->name(), m_plugin->isLogType(type->id()), type->id(), true));
//    }
}

bool LogConfig::processEvent(Event *e)
{
    if ((e->type() == eEventPluginChanged) || (e->type() == eEventLanguageChanged))
        fill();
    return false;
}
