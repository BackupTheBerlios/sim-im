#################
# icons library #
#################
IF(BUILD_DROPPED)
SET(icons_SRCS
        icon.cpp
        iconcfg.cpp
)

SET(icons_HDRS
        icon.h
        iconcfg.h
)

SET(icons_UICS
        iconcfgbase.ui
)

REMOVE_DEFINITIONS(-DQT3_SUPPORT)
REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
SIM_ADD_PLUGIN(icons)
ENDIF(BUILD_DROPPED)
