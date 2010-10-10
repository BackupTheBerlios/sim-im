//
// C++ Interface: BuiltinLogger
//
// Description: Simple built-in logger.
//              Logs to stdout/stderr.
//              Useful when plugins are not loaded.
//
//
// Author: Alexander Petrov <zowers@gmail.com>, (C) 2006
// Created: Sat Feb 25 2006
//
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef SIMBUILTINLOGGER_H
#define SIMBUILTINLOGGER_H

#include <QObject>
#include "simapi.h"
#include "log.h"

namespace SIM {

class EXPORT BuiltinLogger : public QObject
{
    Q_OBJECT
public:
    BuiltinLogger(unsigned logLevel = L_ERROR | L_WARN);
    virtual ~BuiltinLogger();

public slots:
    void logEvent(const QString& message, int logLevel);

private:
    unsigned m_logLevel;
};

}

#endif
