######################
# netmonitor library #
######################
IF(BUILD_DROPPED)
PROJECT(netmonitor)

SET(netmonitor_SRCS
        monitor.cpp
        netmonitor.cpp
)

SET(netmonitor_HDRS
        monitor.h
        netmonitor.h
)

REMOVE_DEFINITIONS(-DQT3_SUPPORT)
REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
SIM_ADD_PLUGIN(netmonitor)
ENDIF(BUILD_DROPPED)
