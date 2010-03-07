##################
# splash library #
##################
IF(BUILD_DROPPED)
SET(splash_SRCS
        splash.cpp
)

SET(splash_HDRS
        splash.h
)

REMOVE_DEFINITIONS(-DQT3_SUPPORT)
REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
SIM_ADD_PLUGIN(splash)

# splash.png
INSTALL(FILES splash.png DESTINATION ${SIM_PICT_DIR})
ENDIF(BUILD_DROPPED)
