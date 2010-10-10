/***************************************************************************
                          cfg.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifndef _CFG_H
#define _CFG_H

#include <QByteArray>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QSharedPointer>

#include "propertyhub.h"
#include "simapi.h"

class QObject;
class QWidget;
class QToolBar;

class Buffer;
class IP;

namespace SIM {

class EXPORT Config
{
public:
    Config(const QString& filename);
    virtual ~Config();

    bool mergeOldConfig(const QString& filename);

    bool addPropertyHub(PropertyHubPtr hub);
    PropertyHubPtr propertyHub(const QString& hubNamespace);
    void clearPropertyHubs();
    PropertyHubPtr rootPropertyHub();

    QByteArray serialize();
    bool deserialize(const QByteArray& arr);
private:
    typedef QMap<QString, PropertyHubPtr> PropertyHubMap;
    PropertyHubMap m_hubs;
    QString m_group;
    QVariantMap m_data;
    QString m_filename;
    PropertyHubPtr m_roothub;
    bool m_changed;
};

typedef QSharedPointer<Config> ConfigPtr;

/* Make directory */
EXPORT bool makedir(const QString &dir);

/* Save state */
EXPORT void save_state();

EXPORT QString getToken(QString &from, char c, bool bUnEsacpe=true);
EXPORT QByteArray getToken(QByteArray &from, char c, bool bUnEsacpe=true);
//EXPORT QByteArray getToken(QByteArray &from, char c, bool bUnEsacpe=true);
EXPORT QString quoteChars(const QString &from, const char *chars, bool bQuoteSlash=true);
EXPORT QString unquoteChars(const QString &from, const QString chars, bool bQuoteSlash = true);

EXPORT char fromHex(char);

} // namespace SIM

#endif
