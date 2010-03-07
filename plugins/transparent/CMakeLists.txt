#######################
# transparent library #
#######################
IF(BUILD_DROPPED)
SET(transparent_SRCS
        transparent.cpp
        transparentcfg.cpp
)

SET(transparent_HDRS
        transparent.h
        transparentcfg.h
)

SET(transparent_UICS
        transparentcfgbase.ui
)

SET(transparent_LIBS
        floaty
)

REMOVE_DEFINITIONS(-DQT3_SUPPORT)
REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
SIM_ADD_PLUGIN(transparent)
ENDIF(BUILD_DROPPED)
