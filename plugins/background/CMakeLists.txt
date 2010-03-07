######################
# background library #
######################
IF(BUILD_DROPPED)
SET(background_SRCS
        background.cpp
        bkgndcfg.cpp
)

SET(background_HDRS
        background.h
        bkgndcfg.h
)

SET(background_UICS
        bkgndcfgbase.ui
)

REMOVE_DEFINITIONS(-DQT3_SUPPORT)
REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
SIM_ADD_PLUGIN(background)
ENDIF(BUILD_DROPPED)
