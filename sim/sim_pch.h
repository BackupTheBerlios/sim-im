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
#include <libxml/parser.h>

#include "QRegExp"

#include "aboutdata.h"
#include "buffer.h"
#include "builtinlogger.h"
#include "cfg.h"
#include "cmddef.h"
#include "contacts.h"
#include "country.h"
#include "event.h"
#include "exec.h"
#include "fetch.h"
#include "icons.h"
#include "sax.h"
#include "singleton.h"
#include "xsl.h"

#include "socket/socket.h"
#include "socket/socketfactory.h"

#include "simgui/ballonmsg.h"
#include "simgui/datepicker.h"
#include "simgui/editfile.h"
#include "simgui/fontedit.h"
#include "simgui/intedit.h"
#include "simgui/linklabel.h"
#include "simgui/listview.h"
#include "simgui/preview.h"
#include "simgui/qchildwidget.h"
#include "simgui/qcolorbutton.h"
#include "simgui/qkeybutton.h"
#include "simgui/textshow.h"
#include "simgui/toolbtn.h"
