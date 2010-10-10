##################
# logger library #
##################
IF(BUILD_DROPPED)
SET(logger_SRCS
        logconfig.cpp
        logger.cpp
)

SET(logger_HDRS
        logconfig.h
        logger.h
)

SET(logger_UICS
        logconfigbase.ui
)

REMOVE_DEFINITIONS(-DQT3_SUPPORT)
REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
SIM_ADD_PLUGIN(logger)
ENDIF(BUILD_DROPPED)
