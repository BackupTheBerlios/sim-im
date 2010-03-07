//
// C++ Implementation: BuiltinLogger
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

#include "builtinlogger.h"

#include <iostream>

namespace SIM {

BuiltinLogger::BuiltinLogger(unsigned logLevel, unsigned priority /*= LowPriority*/)
    : EventReceiver(priority)
    , m_logLevel(logLevel)
{
}

BuiltinLogger::~BuiltinLogger()
{
}

bool BuiltinLogger::processEvent(Event *e)
{
    using namespace std;

    // validate params
    if (!e || e->type() != eEventLog)
        return false;

    EventLog *l = static_cast<EventLog*>(e);

    // filter by log level
    if (!(l->logLevel() & m_logLevel))
        return false;

    // filter out packets: there is LoggerPlugin for packets logging.
    if (l->packetID()) {
        return false;
    }
    cout << "SIM-IM: ";
    if (!l->logData().isEmpty())
    {
        cout << qPrintable(l->logData());
    }
    else
    {
        cout << "Some log event of type " << level_name(l->logLevel()) << " occurred";
    }
    cout << endl;
    return true;
}

}
