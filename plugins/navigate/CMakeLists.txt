####################
# navigate library #
####################
IF(BUILD_DROPPED)
SET(navigate_SRCS
        navcfg.cpp
        navigate.cpp
)

SET(navigate_HDRS
        navcfg.h
        navigate.h
)

SET(navigate_UICS
        navcfgbase.ui
)

ADD_FLEX_FILES(navigate_SRCS parseurl.ll)

REMOVE_DEFINITIONS(-DQT3_SUPPORT)
REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
SIM_ADD_PLUGIN(navigate)
ENDIF(BUILD_DROPPED)
