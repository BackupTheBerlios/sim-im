# main cmake file for SIM-IM

PROJECT(Sim-IM)
SET(Sim-IM_VERSION 0.9.6)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)
SET(CMAKE_COLOR_MAKEFILE ON)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

SET(CMAKE_CXX_FLAGS  "-g -O0 -Wall")
SET(LINK_FLAGS "-g -O0")

IF( APPLE )
    SET( PROGNAME Sim-IM )
    SET( MACOSX_BUNDLE_BUNDLE_NAME Sim-IM )
    SET( MACOSX_BUNDLE_ICON_FILE sim.icns )
    SET( MACOSX_BUNDLE_SHORT_VERSION_STRING 0.9.6 )
    SET( MACOSX_BUNDLE_VERSION 0.9.6 )
    SET( MACOSX_BUNDLE_LONG_VERSION_STRING Version 0.9.6 )
    SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch i386" )
    SET( CMAKE_C_FLAGS "-arch i386" )
ELSE( APPLE )
    SET( PROGNAME sim )
ENDIF( APPLE )

SET(CMAKE_VERBOSE_MAKEFILE OFF)

INCLUDE(Flex)
INCLUDE(Jisp)
INCLUDE(MacroBoolTo01)
INCLUDE(Po)
INCLUDE(SimPlugins)
INCLUDE(DistTarget)
INCLUDE(CheckCXXCompilerFlag)

OPTION(SIM_ENABLE_FPIE "Enable PIE linking")
OPTION(ENABLE_TARGET_UPDATE_MESSAGES "Enables target that allows to extract i18n messages from the code, and update *.po files with them")

# need 4.3 for some QHttp functionality
SET(QT_MIN_VERSION 4.3.0)
FIND_PACKAGE(Qt4 REQUIRED)
IF(NOT QT_FOUND)
    MESSAGE(FATAL_ERROR "Could not find Qt4 - cannot compile Sim-IM here")
ENDIF(NOT QT_FOUND)

# needed packages
FIND_PACKAGE(LibXml2 REQUIRED)
FIND_PACKAGE(LibXslt REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)
FIND_PACKAGE(OpenSSL)
IF(WIN32)
    FIND_PACKAGE(LibIconv REQUIRED)
ENDIF(WIN32)

SET(QT_USE_QTXML 1)
SET(QT_USE_QTUITOOLS 1)
SET(QT_USE_QTNETWORK 1)
SET(QT_USE_PHONON 1)
SET(QT_USE_QTTEST 1)
SET(QT_PHONON_INCLUDE_DIR "/usr/include/phonon")
INCLUDE(${QT_USE_FILE})

# optional packages
FIND_PACKAGE(ASPELL)

IF(NOT WIN32 AND NOT APPLE)
  FIND_PACKAGE(KDE4)
  IF(KDE4_FOUND)
    OPTION(ENABLE_KDE4 "Enable KDE4 support" ON)
    IF(ENABLE_KDE4)
      ADD_DEFINITIONS(${KDE4_DEFINITIONS} -DUSE_KDE4)
      INCLUDE_DIRECTORIES(${KDE4_INCLUDES})
    ENDIF(ENABLE_KDE4)
  ENDIF(KDE4_FOUND)

  INCLUDE(FindX11)
ENDIF(NOT WIN32 AND NOT APPLE)

# do config checks
INCLUDE(ConfigureChecks.cmake)

MACRO_BOOL_TO_01(OPENSSL_FOUND ENABLE_OPENSSL)
MACRO_BOOL_TO_01(X11_FOUND HAVE_X)

# development or release builds
OPTION(DEV_BUILD "Build development version" YES)
OPTION(BUILD_DROPPED "Build dropped plugins" NO)
MACRO_BOOL_TO_01(DEV_BUILD CVS_BUILD)

# global settings for every project
ADD_DEFINITIONS(${QT_DEFINITIONS} -DHAVE_CONFIG_H)
INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}/sim
                    ${Sim-IM_SOURCE_DIR}/sim
                    ${Sim-IM_SOURCE_DIR}/sim/simgui
                    ${Sim-IM_SOURCE_DIR}/plugins/_core)
LINK_DIRECTORIES(${QT_LIB_DIR})

# windows, microsoft compiler
if(MSVC)
  if(CMAKE_COMPILER_2005)
     add_definitions( -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE )
     set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4661" )
  endif(CMAKE_COMPILER_2005)
else(MSVC)
  add_definitions( -Wno-non-virtual-dtor )
endif(MSVC)

add_definitions( -DUNICODE )

IF(CMAKE_COMPILER_IS_GNUCXX AND SIM_ENABLE_FPIE)
    CHECK_CXX_COMPILER_FLAG(-fPIE HAVE_FPIE_SUPPORT)
    IF(HAVE_FPIE_SUPPORT)
        SET (SIM_CXX_FPIE_FLAGS "-fPIE")
        SET (SIM_PIE_LDFLAGS "-pie")
    ELSE(HAVE_FPIE_SUPPORT)
        MESSAGE(STATUS "Your compiler doesn't support PIE flag")
    ENDIF(HAVE_FPIE_SUPPORT)
ENDIF(CMAKE_COMPILER_IS_GNUCXX AND SIM_ENABLE_FPIE)

SET(SIM_FLAVOUR "" CACHE STRING "Additional suffix for plugin dir and executable file")
if(WIN32)
    SET(LIBDIR         .)
    SET(BINDIR         .)
    SET(SIM_I18N_DIR   po)
    SET(SIM_ICONS_DIR  icons)
    SET(SIM_PICT_DIR   pict)
    SET(SIM_PLUGIN_DIR plugins)
    SET(SIM_SOUND_DIR  sounds)
    SET(SIM_STYLES_DIR styles)
else(WIN32)
if(APPLE)
    SET(SIMBUNDLE      ${CMAKE_BINARY_DIR}/sim/${PROGNAME}.app)
    SET(LIBDIR         ${SIMBUNDLE}/Contents/MacOS)
    SET(BINDIR         ${CMAKE_BINARY_DIR})
    SET(SIM_I18N_DIR   ${SIMBUNDLE}/Contents/Resources/po)
    SET(SIM_ICONS_DIR  ${SIMBUNDLE}/Contents/Resources/icons)
    SET(SIM_PICT_DIR   ${SIMBUNDLE}/Contents/Resources/pict)
    SET(SIM_PLUGIN_DIR ${SIMBUNDLE}/Contents/PlugIns)
    SET(SIM_SOUND_DIR  ${SIMBUNDLE}/Contents/Resources/sounds)
    SET(SIM_STYLES_DIR ${SIMBUNDLE}/Contents/Resources/styles)
else(APPLE)
    SET(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)")
    SET(LIBDIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX} CACHE PATH "Path to sim library")
    SET(BINDIR ${CMAKE_INSTALL_PREFIX}/bin CACHE PATH "Path to sim binary")
    SET(DATADIR ${CMAKE_INSTALL_PREFIX}/share/apps/sim CACHE PATH "Path to sim data")
    SET(SIM_PLUGIN_DIR ${LIBDIR}/sim${SIM_FLAVOUR} CACHE PATH "Path to sim plugins")
    SET(SIM_I18N_DIR   ${CMAKE_INSTALL_PREFIX}/share/locale)
    SET(SIM_ICONS_DIR  ${DATADIR}/icons)
    SET(SIM_PICT_DIR   ${DATADIR}/pict)
    SET(SIM_SOUND_DIR  ${DATADIR}/sounds)
    SET(SIM_STYLES_DIR ${DATADIR}/styles)
    SET(SHARE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/share)
    SET(XDG_APPS_DIR   ${SHARE_INSTALL_PREFIX}/applications/kde)
    SET(SERVICES_INSTALL_DIR ${SHARE_INSTALL_PREFIX}/services)
endif(APPLE)
endif(WIN32)	

IF(ENABLE_TARGET_UPDATE_MESSAGES)
 MESSAGE(STATUS "Enabling target update-messages")
  INCLUDE(XGetText)
 
  IF(XGETTEXT_EXECUTABLE AND MSGMERGE_EXECUTABLE)
    MESSAGE(STATUS "Target update-messages successfuly enabled")
  ELSE(XGETTEXT_EXECUTABLE AND MSGMERGE_EXECUTABLE)
    MESSAGE(STATUS "Can't enable target update-messages: one or more componets are missing")
    SET(ENABLE_TARGET_UPDATE_MESSAGES OFF)
  ENDIF(XGETTEXT_EXECUTABLE AND MSGMERGE_EXECUTABLE)
  
ENDIF(ENABLE_TARGET_UPDATE_MESSAGES)

# subdirectories
ADD_SUBDIRECTORY(sim)
SIM_FIND_PLUGINS(${Sim-IM_SOURCE_DIR}/plugins)
SIM_INCLUDE_PLUGINS()

# create config.h
CONFIGURE_FILE(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
add_definitions( -DHAVE_CONFIG_H )

# the (win32) flex generator doesn't care about HAVE_UNISTD_H - it simply has to
# be there. So create a dummy one if none exists
IF(NOT HAVE_UNISTD_H)
    WRITE_FILE( ${CMAKE_CURRENT_BINARY_DIR}/sim/unistd.h
                "/* dummy unistd.h file for flex generated parsers */" )
ENDIF(NOT HAVE_UNISTD_H)

# for uninstall target
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

# uninstall target
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


########################################
# update-messages target
#######################################
IF ( ENABLE_TARGET_UPDATE_MESSAGES )

    ADD_CUSTOM_TARGET(update-messages)

    SET(new_pot "${CMAKE_CURRENT_BINARY_DIR}/new_messages")
    EMPTY_PO_FILE(${new_pot})

    ADD_CUSTOM_COMMAND(TARGET update-messages
        COMMAND echo Processing simlib
    )
    LIST(SORT sim_LIB_MESSAGE_SOURCES)
    FOREACH(file ${sim_LIB_MESSAGE_SOURCES})
        EXTRACT_MESSAGES("${Sim-IM_SOURCE_DIR}/sim/${file}" ${new_pot})
    ENDFOREACH(file)

    LIST(SORT SIM_PLUGINS)
    FOREACH(plugin ${SIM_PLUGINS})
        ADD_CUSTOM_COMMAND(TARGET update-messages
            COMMAND echo Processing plugin ${plugin}
        )
        IF(${plugin}_MESSAGE_SOURCES)
            LIST(SORT ${plugin}_MESSAGE_SOURCES)
            FOREACH(file ${${plugin}_MESSAGE_SOURCES})
                EXTRACT_MESSAGES("${Sim-IM_PLUGINS_SOURCE_DIR}/${plugin}/${file}" ${new_pot})
            ENDFOREACH(file)
        ENDIF(${plugin}_MESSAGE_SOURCES)
    ENDFOREACH(plugin)
    
      ADD_CUSTOM_COMMAND(TARGET update-messages
        COMMAND echo Updating .po files
        COMMAND mv ${CMAKE_SOURCE_DIR}/po/sim.pot ${CMAKE_SOURCE_DIR}/po/sim.pot.backup
        COMMAND mv ${new_pot}.po ${CMAKE_SOURCE_DIR}/po/sim.pot
      )
    
    FILE(GLOB po_files ${CMAKE_SOURCE_DIR}/po/*.po)
    FOREACH(po_file ${po_files})
      ADD_CUSTOM_COMMAND(TARGET update-messages
        COMMAND mv ${po_file} ${po_file}.backup
      )
    ENDFOREACH(po_file)
    
    FOREACH(po_file ${po_files})
      GET_FILENAME_COMPONENT(po_name ${po_file} NAME)
      ADD_CUSTOM_COMMAND(TARGET update-messages
        COMMAND echo ${po_name}
        COMMAND ${MSGMERGE_EXECUTABLE} ${po_file}.backup ${CMAKE_SOURCE_DIR}/po/sim.pot >${po_file}
      )
    ENDFOREACH(po_file)
ENDIF ( ENABLE_TARGET_UPDATE_MESSAGES )

