##################
# action library #
##################
IF(BUILD_DROPPED)
SET(action_SRCS
        action.cpp
        actioncfg.cpp
        additem.cpp
        menucfg.cpp
)

SET(action_HDRS
        action.h
        actioncfg.h
        additem.h
        menucfg.h
)

SET(action_UICS
        actioncfgbase.ui
        additembase.ui
        menucfgbase.ui
)

REMOVE_DEFINITIONS(-DQT3_SUPPORT)
REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
SIM_ADD_PLUGIN(action)
ENDIF(BUILD_DROPPED)
