#####################
# shortcuts library #
#####################
IF(BUILD_DROPPED)
SET(shortcuts_SRCS
        mousecfg.cpp
        shortcutcfg.cpp
        shortcuts.cpp
)

SET(shortcuts_HDRS
        mousecfg.h
        shortcutcfg.h
        shortcuts.h
)

SET(shortcuts_UICS
        mousecfgbase.ui
        shortcutcfgbase.ui
)

SIM_ADD_PLUGIN(shortcuts)
ENDIF(BUILD_DROPPED)
