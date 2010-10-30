#################
# _core library #
#################
ADD_SUBDIRECTORY(pict)
ADD_SUBDIRECTORY(styles)

SET(_core_SRCS
#        arcfg.cpp
#        autoreply.cpp
#        cfgdlg.cpp
#        cmenu.cpp
#        commands.cpp
#        connectionsettings.cpp
#        connectwnd.cpp
#        containermanager.cpp
#        container.cpp
        core.cpp
        commonstatus.cpp
#        declinedlg.cpp
#        editmail.cpp
#        editphone.cpp
#        filecfg.cpp
#        filetransfer.cpp
#        history.cpp
#        historycfg.cpp
#        historywnd.cpp
#        interfacecfg.cpp
        profileselectdialog.cpp
#        maininfo.cpp
        mainwin.cpp
#        manager.cpp
#        msgauth.cpp
#        msgcfg.cpp
#        msgcontacts.cpp
#        msgedit.cpp
#        msgfile.cpp
#        msggen.cpp
#        msgrecv.cpp
#        msgsms.cpp
#        msgurl.cpp
#        msgview.cpp
#        msgview_menu.cpp
#        newprotocol.cpp
#        nonim.cpp
#        pagerdetails.cpp
#        phonedetails.cpp
#        plugincfg.cpp
#        prefcfg.cpp
#        search.cpp
#        searchall.cpp
#        smscfg.cpp
#        status.cpp
#        statuswnd.cpp
#        textedit_menu.cpp
#        tmpl.cpp
#        toolbar_container.cpp
#        toolbar_history.cpp
#        toolbar_main.cpp
#        toolbar_msgedit.cpp
#        toolbar_textedit.cpp
#        toolbarcfg.cpp
#        toolsetup.cpp
#        usercfg.cpp
#        userhistorycfg.cpp
#        userlist.cpp
#        userviewcfg.cpp
#        userwnd.cpp
        roster/userview.cpp
        roster/userviewmodel.cpp
#        roster/userviewdelegate.cpp
)

# is this really needed?
IF(NOT WIN32)
SET(_core_SRCS
        ${_core_SRCS}
        libintl.cpp
    )
ENDIF(NOT WIN32)

SET(_core_HDRS
#        arcfg.h
#        autoreply.h
#        cfgdlg.h
#        cmenu.h
#        commands.h
#        connectionsettings.h
#        connectwnd.h
#        containermanager.h
#        container.h
        core.h
        core_api.h
        commonstatus.h
#        core_consts.h
#        core_events.h
#        declinedlg.h
#        editmail.h
#        editphone.h
#        filecfg.h
#        filetransfer.h
#        history.h
#        historycfg.h
#        historywnd.h
#        interfacecfg.h
        profileselectdialog.h
#        maininfo.h
        mainwin.h
#        manager.h
#        msgauth.h
#        msgcfg.h
#        msgcontacts.h
#        msgedit.h
#        msgfile.h
#        msggen.h
#        msgrecv.h
#        msgsms.h
#        msgurl.h
#        msgview.h
#        newprotocol.h
#        nonim.h
#        pagerdetails.h
#        phonedetails.h
#        plugincfg.h
#        prefcfg.h
#        search.h
#        searchall.h
#        smscfg.h
#        status.h
#        statuswnd.h
#        tmpl.h
#        toolbarcfg.h
#        toolsetup.h
#        usercfg.h
#        userhistorycfg.h
#        userlist.h
#        userviewcfg.h
#        userwnd.h
        roster/userview.h
        roster/userviewmodel.h
#        roster/userviewdelegate.h
)

SET(_core_UICS
#        arcfgbase.ui
#        autoreplybase.ui
#        cfgdlgbase.ui
#        connectionsettingsbase.ui
#        connectwndbase.ui
#        declinedlgbase.ui
#        editmailbase.ui
#        editphonebase.ui
#        filecfgbase.ui
#        filetransferbase.ui
#        fontconfigbase.ui
#        historycfgbase.ui
#        interfacecfgbase.ui
        profileselectdialog.ui
#        maininfobase.ui
#        managerbase.ui
#        msgcfgbase.ui
#        newprotocolbase.ui
#        nonimbase.ui
#        pagerbase.ui
#        phonebase.ui
#        plugincfgbase.ui
#        prefcfgbase.ui
#        searchallbase.ui
#        searchbase.ui
#        smscfgbase.ui
#        toolsetupbase.ui
#        userhistorycfgbase.ui
#        userviewcfgbase.ui
)

SET(core_test_sources
        tests/test.cpp
        tests/testcoreplugin.cpp
        tests/testcommonstatus.cpp
        tests/testuserviewmodel.cpp
)

ADD_EXECUTABLE(test_core ${core_test_sources})
TARGET_LINK_LIBRARIES(test_core simlib ${QT_LIBRARIES} gtest gmock _core)

# so we don't need an extra CMakeLists.txt in every subdirectory
ADD_JISP_ARCHIVE(jisp       sim.jisp        _core_SRCS)

IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/jisp/icondef.xml)
    message( "Using non-GPL icons" )
    ADD_JISP_ARCHIVE(GPL-Icons  GPL-Icons.jisp  _core_SRCS)
ELSE (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/jisp/icondef.xml)
    message( "Using GPL icons" )
    # substitute with gpl icons
    ADD_JISP_ARCHIVE(GPL-Icons  sim.jisp        _core_SRCS)
ENDIF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/jisp/icondef.xml)

ADD_JISP_ARCHIVE(smiles     smiles.jisp     _core_SRCS)
ADD_JISP_ARCHIVE(additional additional.jisp _core_SRCS)
ADD_JISP_ARCHIVE(icq5       icq5.jisp       _core_SRCS)
ADD_JISP_ARCHIVE(icq5.1     icq5.1.jisp     _core_SRCS)
ADD_JISP_ARCHIVE(icqlite    icqlite.jisp    _core_SRCS)
ADD_JISP_ARCHIVE(qip-icons  qip-icons.jisp  _core_SRCS)
ADD_JISP_ARCHIVE(lovenmoney lovenmoney.jisp _core_SRCS)
ADD_JISP_ARCHIVE(yahoo      yahoo.jisp      _core_SRCS)
ADD_JISP_ARCHIVE(msn        msn.jisp        _core_SRCS)

FILE(GLOB _prebuilt emoticons_prebuilt/*.jisp)

IF(APPLE)
    FOREACH(FILE ${_prebuilt})
        GET_FILENAME_COMPONENT(FILENAME ${FILE} NAME)
        CONFIGURE_FILE(${FILE} ${SIM_ICONS_DIR}/${NAME} COPYONLY)
    ENDFOREACH(FILE)
ENDIF(APPLE)

INCLUDE_DIRECTORIES(../${CMAKE_CURRENT_SOURCE_DIR})

INSTALL(FILES ${_prebuilt} DESTINATION ${SIM_ICONS_DIR})

SIM_ADD_PLUGIN(_core)

SET_TARGET_PROPERTIES(_core PROPERTIES DEFINE_SYMBOL CORE_EXPORTS)

