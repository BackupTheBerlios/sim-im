###################
# forward library #
###################
IF(BUILD_DROPPED)
SET(forward_SRCS
        forward.cpp
        forwardcfg.cpp
)

SET(forward_HDRS
        forward.h
        forwardcfg.h
)

SET(forward_UICS
        forwardcfgbase.ui
)

SET(forward_LIBS
        _core
)

REMOVE_DEFINITIONS(-DQT3_SUPPORT)
REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
SIM_ADD_PLUGIN(forward)
ENDIF(BUILD_DROPPED)

