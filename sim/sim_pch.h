/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#pragma once

#include <windows.h>
#include <process.h>

#include <qglobal.h>

#include <QApplication>
#include <QString>
#include <QBitmap>
#include <QPushButton>
#include <QClipboard>
#include <QCursor>
#include <QDialog>
#include <QFile>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QLineedit>
#include <QPainter>
#include <QColorGroup>
#include <QObject>
#include <QMenu>
#include <QPushbutton>
#include <QSocketnotifier>
#include <QString>
#include <QStringlist>
#include <QTabbar>
#include <QTabwidget>
#include <QThread>
#include <QTimer>
#include <QTooltip>
#include <QValidator>

//Fixme:
//#ifndef NO_QT_MOC_HEADER
#include <QMetaObject>
/*#include <private/qucomextra_p>
#include <QSignalslotimp>
#endif*/

#ifdef ENABLE_OPENSSL
	#include <openssl/ssl.h>
	#include <openssl/err.h>
	#include <openssl/bio.h>
	#include <openssl/rand.h>
#endif
//#include <libxml/parser.h">

#include "QRegExp"

#include "aboutdata.h"
#include "buffer.h"
#include "builtinlogger.h"
#include "cfg.h"
//#include "cjk_variants.h"
#include "clientmanager.h"
#include "contacts.h"
#include "country.h"
#include "exec.h"
//#include "johab_hangul.h"
#include "kdeisversion.h"
#include "log.h"
#include "misc.h"
#include "paths.h"
#include "plugins.h"
#include "profile.h"
#include "profilemanager.h"
#include "propertyhub.h"
#include "simapi.h"
#include "simfs.h"
#include "singleton.h"
#include "standardclientmanager.h"
//#include "translit.h"

#include "commands/commandhub.h"
#include "commands/commandset.h"
#include "commands/uicommand.h"

#include "contacts/client.h"
#include "contacts/clientdatastruct.h"
#include "contacts/clientuserdata.h"
#include "contacts/contact.h"
#include "contacts/contactlist.h"
#include "contacts/group.h"
#include "contacts/imcontact.h"
#include "contacts/imgroup.h"
#include "contacts/imstatus.h"
#include "contacts/imstatusinfo.h"
#include "contacts/packettype.h"
#include "contacts/protocol.h"
#include "contacts/protocolmanager.h"
#include "contacts/standardcontactlist.h"
#include "contacts/userdata.h"

#include "events/eventdata.h"
#include "events/eventhub.h"
#include "events/ievent.h"
#include "events/logevent.h"
#include "events/nulleventdata.h"
#include "events/standardevent.h"

#include "imagestorage/iconset.h"
#include "imagestorage/imagestorage.h"
#include "imagestorage/jispiconset.h"
#include "imagestorage/standardimagestorage.h"

#include "libltdl/ltdl.h"

#include "messaging/genericmessage.h"
#include "messaging/message.h"
#include "messaging/messagepipe.h"
#include "messaging/messageprocessor.h"
#include "messaging/standardmessagepipe.h"

#include "network/asyncsocket.h"
#include "network/tcpasyncsocket.h"

//#include "simapp/kdesimapp.h"
#include "simapp/simapp.h"

#include "simgui/ballonmsg.h"
#include "simgui/intedit.h"
#include "simgui/linklabel.h"
#include "simgui/preview.h"
#include "simgui/qchildwidget.h"
#include "simgui/qcolorbutton.h"
#include "simgui/qkeybutton.h"
#include "simgui/standardtoolbaractionfactory.h"
#include "simgui/statuswidget.h"
#include "simgui/toolbar.h"
#include "simgui/toolbaractionfactory.h"
#include "simgui/toolbarbutton.h"
#include "simgui/toolbarcombobox.h"
#include "simgui/toolbutton.h"
#include "simgui/toolitem.h"
