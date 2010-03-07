#pragma once


#include <qglobal.h>

#include <QApplication>
#include <QBitmap>
#include <QClipboard>
#include <QCursor>
#include <QDialog>
#include <QFile>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QObject>
#include <QObject>
#include <QPainter>
#include <QPushButton>
#include <QSocketNotifier>
#include <QString>
#include <QStringList>
#include <QTabBar>
#include <QTabWidget>
#include <QThread>
#include <QTimer>
#include <QToolTip>
#include <QValidator>

#ifndef NO_QT_MOC_HEADER
#include <qmetaobject.h>
//#include <private/qucomextra_p.h>
//#include <qsignalslotimp.h>
#endif

#include "gpg.h"
#include "gpgadv.h"
#include "ui_gpgadvbase.h"
#include "gpgcfg.h"
#include "ui_gpgcfgbase.h"
#include "gpgfind.h"
#include "ui_gpgfindbase.h"
#include "gpggen.h"
#include "ui_gpggenbase.h"
#include "passphrase.h"
#include "ui_passphrasebase.h"
