/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _SIMAPI_H
#define _SIMAPI_H

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

// for Q_CC_foo - macros
#include <qglobal.h>

#ifdef Q_CC_MSVC
	// "conditional expression is constant" (W4)
	# pragma warning(disable: 4127)
	// "'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'" (W1)
	# pragma warning(disable: 4251)
	// "function' : function not inlined" (W4)4786
	# pragma warning(disable: 4710)
	// "function' : assignment operator could not be generated" 
	# pragma warning(disable: 4512) //Attention should perhaps not be pragma'ed
#endif

#define SIM_EXPORT Q_DECL_EXPORT
#define SIM_IMPORT Q_DECL_IMPORT

#ifdef SIMAPI_EXPORTS
        # define EXPORT SIM_EXPORT
#else
        # define EXPORT SIM_IMPORT
#endif

#ifdef __OS2__
        # define EXPORT_PROC extern "C" _System
#else
        # define EXPORT_PROC extern "C" SIM_EXPORT
#endif

#if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#define SIM_DEPRECATED  __attribute__((__deprecated__))
#else
#define SIM_DEPRECATED
#endif /* __GNUC__ */

# define DEPRECATED Q_DECL_DEPRECATED

#ifndef COPY_RESTRICTED
	# define COPY_RESTRICTED(A) \
    private: \
        A(const A&); \
        A &operator = (const A&);
#endif

#endif // _SIMAPI_H
