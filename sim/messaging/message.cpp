/***************************************************************************
                          message.cpp  -  description
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

#include <time.h>
#include <vector>

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>

#include "message.h"
#include "misc.h"

namespace SIM
{
    void Message::setFlag(Flag fl, bool value)
    {
        m_flags.setBit(fl, value);
    }

    bool Message::flag(Flag fl)
    {
        return m_flags.at(fl);
    }
}

