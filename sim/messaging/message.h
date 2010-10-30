/***************************************************************************
                          message.h  -  description
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

#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <QString>
#include <QDateTime>
#include <QBitArray>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QIcon>

#include "simapi.h"
#include "cfg.h"

namespace SIM {

class Client;
typedef QWeakPointer<Client> ClientWeakPtr;

class IMContact;
typedef QWeakPointer<IMContact> IMContactWeakPtr;

class EXPORT Message : public QObject
{
    Q_OBJECT
public:
    enum Flag
    {
        flMaxFlag
    };

    virtual ~Message() {}

    virtual Client* client() = 0;
    virtual IMContactWeakPtr contact() const = 0;

    virtual QIcon icon() = 0;
    virtual QDateTime timestamp() = 0;
    virtual QString toHtml() = 0;
    virtual QString toPlainText() = 0;
    virtual QString originatingClientId() const = 0;
    virtual QStringList choices() = 0;

    void setFlag(Flag fl, bool value);
    bool flag(Flag fl);

signals:
    void choice(const QString& choiceId);

private:
    QBitArray m_flags;
};

typedef QSharedPointer<Message> MessagePtr;


} // namespace SIM

#endif
