################
# dock library #
################
IF(BUILD_DROPPED)
SET(dock_SRCS
        dock.cpp
        dockcfg.cpp
        dockwnd.cpp
)

SET(dock_HDRS
        dock.h
        dockcfg.h
        dockwnd.h
)

SET(dock_UICS
        dockcfgbase.ui
)

SIM_ADD_PLUGIN(dock)
ENDIF(BUILD_DROPPED)
