###############
# osd library #
###############
SET(osd_SRCS
        osd.cpp
        osdconfig.cpp
        osdiface.cpp
)

SET(osd_HDRS
        osd.h
        osdconfig.h
        osdiface.h
)

SET(osd_UICS
        osdconfigbase.ui
        osdifacebase.ui
)

REMOVE_DEFINITIONS(-DQT3_SUPPORT)
REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
SIM_ADD_PLUGIN(osd)
