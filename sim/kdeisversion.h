/***************************************************************************
                        kdeisversion.h  -  description
                             -------------------
    begin                : Wed Feb 18 2004
    copyright            : (C) 2004 by Marcel Meckel
    email                : debian@thermoman.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KDEISVERSION_H
#define _KDEISVERSION_H

#ifdef USE_KDE
#include <kapplication.h>

// Emulate KDE_IS_VERSION for older KDE Versions

#if !defined(KDE_IS_VERSION)
#define KDE_MAKE_VERSION( a,b,c ) (((a) << 16) | ((b) << 8) | (c))
#undef KDE_VERSION
#define KDE_VERSION KDE_MAKE_VERSION(KDE_VERSION_MAJOR,KDE_VERSION_MINOR,KDE_VERSION_RELEASE)
#define KDE_IS_VERSION(a,b,c) ( KDE_VERSION >= KDE_MAKE_VERSION(a,b,c) )
#endif

#endif
#endif

