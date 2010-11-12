PROJECT(sim)

IF(NOT WIN32)
    ADD_SUBDIRECTORY(png)
ENDIF(NOT WIN32)

######################
# target sim library #
######################

SET(sim_LIB_SRCS
		buffer.cpp
		builtinlogger.cpp
		cfg.cpp
		clientmanager.cpp
        standardclientmanager.cpp
        commands/commandhub.cpp
        commands/commandset.cpp
        commands/uicommand.cpp
		country.cpp
        events/eventdata.cpp
        events/eventhub.cpp
        events/nulleventdata.cpp
        events/standardevent.cpp
        events/ievent.cpp
        events/logevent.cpp
		exec.cpp
        imagestorage/imagestorage.cpp
        imagestorage/standardimagestorage.cpp
        imagestorage/iconset.cpp
        imagestorage/jispiconset.cpp
		log.cpp
        messaging/message.cpp
        messaging/genericmessage.cpp
        messaging/messagepipe.cpp
        messaging/standardmessagepipe.cpp
        messaging/messageprocessor.cpp
		misc.cpp
		paths.cpp
		plugins.cpp
		profile.cpp
		profilemanager.cpp
		propertyhub.cpp
		simfs.cpp
		translit.cpp
		qzip/ioapi.c
		qzip/qzip.cpp
		qzip/unzip.c
		contacts/userdata.cpp
		contacts/packettype.cpp
		contacts/contact.cpp
        contacts/contactlist.cpp
        contacts/standardcontactlist.cpp
		contacts/group.cpp
		contacts/protocol.cpp
		contacts/protocolmanager.cpp
		contacts/client.cpp
		contacts/imstatus.cpp
        contacts/imcontact.cpp
        contacts/imgroup.cpp

        network/asyncsocket.cpp
        network/tcpasyncsocket.cpp

        simgui/ballonmsg.cpp
		simgui/linklabel.cpp
		simgui/preview.cpp
		simgui/qchildwidget.cpp
		simgui/qcolorbutton.cpp
		simgui/qkeybutton.cpp
        simgui/toolbarbutton.cpp
        simgui/toolbarcombobox.cpp
        simgui/toolbar.cpp
        simgui/toolbaractionfactory.cpp
        simgui/standardtoolbaractionfactory.cpp
        simgui/statuswidget.cpp
        simgui/toolitem.cpp
	)

SET(sim_LIB_MESSAGE_SOURCES  ${sim_LIB_SRCS} PARENT_SCOPE)

SET(sim_LIB_HDRS
		buffer.h
		builtinlogger.h
		cfg.h
		clientmanager.h
        standardclientmanager.h
        commands/commandhub.h
        commands/commandset.h
        commands/uicommand.h
		country.h
        events/eventdata.h
        events/eventhub.h
        events/nulleventdata.h
        events/standardevent.h
        events/ievent.h
        events/logevent.h
		exec.h
        imagestorage/imagestorage.h
        imagestorage/standardimagestorage.h
        imagestorage/iconset.h
        imagestorage/jispiconset.h
		log.h
        messaging/message.h
        messaging/genericmessage.h
        messaging/messagepipe.h
        messaging/standardmessagepipe.h
        messaging/messageprocessor.h
		misc.h
		paths.h
		plugins.h
		profile.h
		profilemanager.h
		propertyhub.h
		simapi.h
		simfs.h
		singleton.h
		translit.h
		qzip/ioapi.h
		qzip/qzip.h
		qzip/unzip.h
		contacts/userdata.h
		contacts/packettype.h
		contacts/contact.h
        contacts/contactlist.h
        contacts/standardcontactlist.h
		contacts/group.h
		contacts/protocol.h
		contacts/protocolmanager.h
		contacts/client.h
		contacts/imstatus.h
        contacts/imcontact.h
        contacts/imgroup.h

        network/asyncsocket.h
        network/tcpasyncsocket.h

		simgui/ballonmsg.h
		simgui/linklabel.h
		simgui/preview.h
		simgui/qchildwidget.h
		simgui/qcolorbutton.h
		simgui/qkeybutton.h
        simgui/toolbarbutton.h
        simgui/toolbarcombobox.h
        simgui/toolbar.h
        simgui/toolbaractionfactory.h
        simgui/standardtoolbaractionfactory.h
        simgui/statuswidget.h
        simgui/toolitem.h
	)

#SET(sim_ui
#	simgui/logindlgbase.ui)

IF(ENABLE_KDE4)
	SET(sim_LIB_SRCS ${sim_LIB_SRCS} simapp/kdesimapp.cpp)
	SET(sim_LIB_HDRS ${sim_LIB_HDRS} simapp/kdesimapp.h)
ELSE(ENABLE_KDE4)
	SET(sim_LIB_SRCS ${sim_LIB_SRCS} simapp/simapp.cpp aboutdata.cpp)
	SET(sim_LIB_HDRS ${sim_LIB_HDRS} simapp/simapp.h aboutdata.h)
ENDIF(ENABLE_KDE4)

# moc the sources
QT4_WRAP_CPP(MOC_SRCS ${sim_LIB_HDRS})
QT4_WRAP_UI(sim_ui_headers ${sim_ui})
SOURCE_GROUP(MOC-Files FILES ${MOC_SRCS})

# don't forget flex
#ADD_FLEX_FILES(sim_LIB_SRCS html.ll)

# our target
ADD_LIBRARY(simlib SHARED ${sim_LIB_SRCS} ${sim_LIB_HDRS} ${MOC_SRCS} ${sim_ui_headers})

IF(WIN32 AND NOT MINGW)
    SET_TARGET_PROPERTIES(simlib PROPERTIES VERSION 0.0.0 SOVERSION 0)
ELSE(WIN32 AND NOT MINGW)
    SET_TARGET_PROPERTIES(simlib PROPERTIES OUTPUT_NAME sim${SIM_FLAVOUR} VERSION 0.0.0 SOVERSION 0)
ENDIF(WIN32 AND NOT MINGW)

#some needed defines
SET_TARGET_PROPERTIES(simlib PROPERTIES DEFINE_SYMBOL SIMAPI_EXPORTS)
ADD_DEFINITIONS(-DNOUNCRYPT)

# additional include dirs - at least ${CMAKE_CURRENT_BINARY_DIR} is needed
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(simlib)

IF(ENABLE_KDE4)
  TARGET_LINK_LIBRARIES(simlib ${KDE4_KDECORE_LIBRARY} ${KDE4_KDEUI_LIBRARY} ${KDE4_KIO_LIBRARY})
ENDIF(ENABLE_KDE4)

# we need wsock32 on win32
if (WIN32)
  TARGET_LINK_LIBRARIES(simlib wsock32 wininet)
endif(WIN32)

# libxml, libxslt and iconv
INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR} ${LIBICONV_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(simlib ${QT_LIBRARIES} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} ${ZLIB_LIBRARIES})

IF(WIN32)
  TARGET_LINK_LIBRARIES(simlib ${LIBICONV_LIBRARIES})
ENDIF(WIN32)

# set correct install path
INSTALL(TARGETS simlib LIBRARY DESTINATION ${LIBDIR} RUNTIME DESTINATION ${LIBDIR} BUNDLE DESTINATION ${LIBDIR})

IF(WIN32)
    SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
ENDIF(WIN32)

IF(APPLE)
    SET(LIBRARY_OUTPUT_PATH ${LIBDIR})
ENDIF(APPLE)

# unit tests
SET(sim_tests_srcs
    tests/test.cpp
    tests/testbuffer.cpp
    tests/testclientmanager.cpp
    tests/testcommandhub.cpp
    tests/testcommandset.cpp
    tests/testuicommand.cpp
    tests/testtoolbar.cpp
    tests/teststandardtoolbaractionfactory.cpp
    tests/testcontact.cpp
    tests/testgroup.cpp
    tests/testmessagepipe.cpp
    tests/teststandardcontactlist.cpp
)


SET(sim_tests_hdrs
    tests/mocks/mockclient.h
    tests/mockprotocol.h
    tests/stubs/stubclient.h
    tests/stubs/stubimagestorage.h
    tests/stubs/stubimcontact.h
    tests/stubs/stubimgroup.h
    tests/mocks/mockcontactlist.h
    tests/mocks/mockimcontact.h
    tests/mocks/mockasyncsocket.h
    tests/testuicommand.h
    tests/gtest-qt.h
)

QT4_WRAP_CPP(testmocs ${sim_tests_hdrs})

ADD_EXECUTABLE(test ${sim_tests_srcs} ${testmocs})
TARGET_LINK_LIBRARIES(test simlib ${QT_LIBRARIES} ${sim_ui_headers} gtest gmock)

##################
# sim executable #
##################

# main program
SET(sim_EXE_SRCS
    sim.cpp
)

if(WIN32)
  if(MINGW)
    # resource compilation for mingw
    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sim_rc.o
                       COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR}
                                           -i${CMAKE_CURRENT_SOURCE_DIR}/sim.rc
                                           -o ${CMAKE_CURRENT_BINARY_DIR}/sim_rc.o)
    SET(sim_EXE_SRCS ${sim_EXE_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/sim_rc.o)
  else(MINGW)
    SET(sim_EXE_SRCS ${sim_EXE_SRCS} sim.rc)
  endif(MINGW)
endif(WIN32)

# i18n
COMPILE_PO_FILES(${CMAKE_SOURCE_DIR}/po sim_EXE_SRCS)

# sim executable
ADD_EXECUTABLE(${PROGNAME} WIN32 MACOSX_BUNDLE ${sim_EXE_SRCS})

IF(SIM_ENABLE_FPIE)
    SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES COMPILE_FLAGS "${SIM_CXX_FPIE_FLAGS}")
    SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES LINK_FLAGS "${SIM_PIE_LDFLAGS}")
ENDIF(SIM_ENABLE_FPIE)

# only needs simlib
TARGET_LINK_LIBRARIES(${PROGNAME} simlib ${QT_LIBRARIES} ${QT_QTMAIN_LIBRARY})

# set correct install path
INSTALL(TARGETS ${PROGNAME} RUNTIME DESTINATION ${BINDIR} BUNDLE DESTINATION ${BINDIR})

SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES OUTPUT_NAME ${PROGNAME}${SIM_FLAVOUR})

IF(WIN32)
    SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
ENDIF(WIN32)

IF(NOT WIN32 AND NOT APPLE)
    INSTALL(FILES sim.desktop DESTINATION ${XDG_APPS_DIR})
ENDIF(NOT WIN32 AND NOT APPLE)

# some needed libs for win32
if(WIN32)
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install_win32/iconv.dll    DESTINATION ${BINDIR})
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install_win32/libeay32.dll DESTINATION ${BINDIR})
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install_win32/libxml2.dll  DESTINATION ${BINDIR})
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install_win32/libxslt.dll  DESTINATION ${BINDIR})
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install_win32/ssleay32.dll DESTINATION ${BINDIR})
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install_win32/zlib1.dll    DESTINATION ${BINDIR})
endif(WIN32)
