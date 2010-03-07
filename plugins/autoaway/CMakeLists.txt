IF(BUILD_DROPPED)
IF(NOT WIN32 AND NOT APPLE)
    INCLUDE(CheckFunctionExists)
    INCLUDE(FindX11)
    INCLUDE(CheckLibraryExists)
    #SET(CMAKE_REQUIRED_LIBRARIES "X11 Xext m")
    CHECK_LIBRARY_EXISTS("Xext" "XScreenSaverRegister" "${X11_LIBRARY_DIR}" XSS_IN_XEXT)
    IF(XSS_IN_XEXT)
        SET(XSS_LIB "Xext")
    ELSE(XSS_IN_XEXT)
        CHECK_LIBRARY_EXISTS("Xss" "XScreenSaverRegister" "${X11_LIBRARY_DIR}" XSS_IN_XSS)
        IF(XSS_IN_XSS)
            SET(XSS_LIB "Xss")
        ENDIF(XSS_IN_XSS)
    ENDIF(XSS_IN_XEXT)
    IF(XSS_LIB)
        CHECK_INCLUDE_FILES("X11/extensions/scrnsaver.h" HAVE_X11_EXTENSIONS_SCRNSAVER)
    ENDIF(XSS_LIB)
    IF(HAVE_X11_EXTENSIONS_SCRNSAVER)
        SET(COMPILE_PLUGIN TRUE)
    ELSE(HAVE_X11_EXTENSIONS_SCRNSAVER)
        IF( X11_Xscreensaver_FOUND )
            SET(COMPILE_PLUGIN TRUE)
        ENDIF( X11_Xscreensaver_FOUND )
    ENDIF(HAVE_X11_EXTENSIONS_SCRNSAVER)
ELSE(NOT WIN32 AND NOT APPLE)
    SET(COMPILE_PLUGIN TRUE)
ENDIF(NOT WIN32 AND NOT APPLE)

IF(COMPILE_PLUGIN)
    ####################
    # autoaway library #
    ####################
    SET(autoaway_SRCS
            autoaway.cpp
            autoawaycfg.cpp
    )

    SET(autoaway_HDRS
            autoaway.h
            autoawaycfg.h
    )

    SET(autoaway_UICS
            autoawaycfgbase.ui
    )

    SET(autoaway_LIBS
            _core
    )

    IF(WIN32)
        SET(autoaway_SRCS
            ${autoaway_SRCS}
            idleui.cpp
        )
        SET(autoaway_HDRS
            ${autoaway_HDRS}
            idleui.h
        )
    ENDIF(WIN32)

    LINK_DIRECTORIES(${X11_LIBRARY_DIR})
    INCLUDE_DIRECTORIES(${X11_INCLUDE_DIR})
    SIM_ADD_PLUGIN(autoaway)
    TARGET_LINK_LIBRARIES(autoaway ${XSS_LIB})
ELSE(COMPILE_PLUGIN)
    MESSAGE(STATUS "XScreenSaver library not found, autoaway plugin disabled")
ENDIF(COMPILE_PLUGIN)
ENDIF(BUILD_DROPPED)
