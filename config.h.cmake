// version
#define PACKAGE "sim"
#define VERSION "0.9.6"
#define PREFIX "${CMAKE_INSTALL_PREFIX}"
#define PLUGIN_PATH "${SIM_PLUGIN_DIR}"

// header
#cmakedefine HAVE_CARBON_CARBON_H 1
#cmakedefine HAVE_INTTYPES_H 1
#cmakedefine HAVE_STDDEF_H 1
#cmakedefine HAVE_STDINT_H 1
#cmakedefine HAVE_STDLIB_H 1
#cmakedefine HAVE_STRING_H 1
#cmakedefine HAVE_STRCASECMP 1
#cmakedefine HAVE_SYS_STAT_H 1
#cmakedefine HAVE_SYS_TYPES_H 1
#cmakedefine HAVE_UNISTD_H 1

// symbols
#cmakedefine HAVE_STRNCASECMP 1

// functions
#cmakedefine HAVE_CHMOD 1
#cmakedefine HAVE_MMAP 1
#cmakedefine HAVE_MUNMAP 1
#cmakedefine HAVE_UNAME 1

// check for structure member
#cmakedefine HAVE_TM_GMTOFF 1

// libraries
#cmakedefine ENABLE_OPENSSL 1
#cmakedefine HAVE_X 1
#cmakedefine USE_KDE 1

// development builds
#cmakedefine CVS_BUILD 1
