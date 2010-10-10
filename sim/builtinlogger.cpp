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

BuiltinLogger::BuiltinLogger(unsigned logLevel) : QObject(0), m_logLevel(logLevel)
{
}

BuiltinLogger::~BuiltinLogger()
{
}

void BuiltinLogger::logEvent(const QString& message, int logLevel)
{
    if(logLevel & m_logLevel)
    {
        std::cerr << qPrintable(message) << std::endl;
    }
}

}
