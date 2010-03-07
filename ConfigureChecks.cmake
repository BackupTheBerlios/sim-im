INCLUDE(CheckIncludeFiles)
INCLUDE(CheckSymbolExists)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckStructMember)

# FIXME: Please check if this is really needed!
# HAVE_GCC_VISIBILITY missing

# Header
CHECK_INCLUDE_FILES(Carbon/Carbon.h HAVE_CARBON_CARBON_H)               # autoaway.cpp, do we support mac?
CHECK_INCLUDE_FILES(inttypes.h      HAVE_INTTYPES_H)                    # simapi.h
CHECK_INCLUDE_FILES(stddef.h        HAVE_STDDEF_H)                      # simapi.h
CHECK_INCLUDE_FILES(stdint.h        HAVE_STDINT_H)                      # simapi.h
CHECK_INCLUDE_FILES(stdlib.h        HAVE_STDLIB_H)                      # simapi.h
CHECK_INCLUDE_FILES(string.h        HAVE_STRING_H)                      # _core/libintl.cpp
CHECK_INCLUDE_FILES(sys/stat.h      HAVE_SYS_STAT_H)                    # gpg/gpg.cpp
CHECK_INCLUDE_FILES(sys/types.h     HAVE_SYS_TYPES_H)                   # simapi.h
CHECK_INCLUDE_FILES(unistd.h        HAVE_UNISTD_H)                      # simapi.h
CHECK_INCLUDE_FILES(xutility        HAVE_XUTILITY)                      # simapi.h

# Symbols
CHECK_SYMBOL_EXISTS(strcasecmp  "strings.h"         HAVE_STRCASECMP)    # simapi.h, various

# Functions
IF(NOT WIN32) # there is a chmod function on win32, but not usable the way we want...
  CHECK_FUNCTION_EXISTS(chmod     HAVE_CHMOD)         # __homedir/homedir.cpp, gpg/gpg.cpp
ENDIF(NOT WIN32)
CHECK_FUNCTION_EXISTS(mmap      HAVE_MMAP)          # _core/libintl.cpp
CHECK_FUNCTION_EXISTS(munmap    HAVE_MUNMAP)        # _core/libintl.cpp
CHECK_FUNCTION_EXISTS(uname     HAVE_UNAME)         # sim/fetch.cpp

# check for structure member
CHECK_STRUCT_MEMBER(tm tm_gmtoff time.h HAVE_TM_GMTOFF)        # icqclient.cpp
