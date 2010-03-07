##################
# floaty library #
##################
IF(BUILD_DROPPED)
SET(floaty_SRCS
        floaty.cpp
        floatywnd.cpp
)

SET(floaty_HDRS
        floaty.h
        floatywnd.h
)

SET(floaty_LIBS
        _core
)

REMOVE_DEFINITIONS(-DQT3_SUPPORT)
REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
SIM_ADD_PLUGIN(floaty)
ENDIF(BUILD_DROPPED)
