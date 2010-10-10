#####################
# __homedir library #
#####################

IF(BUILD_DROPPED)

#conditional sources
IF(WIN32)
	SET (__homedir_SRCS homedircfg.cpp)
	SET (__homedir_HDRS homedircfg.h)
	SET (__homedir_UICS homedircfgbase.ui)
ENDIF(WIN32)

SET(__homedir_SRCS
        homedir.cpp
	${__homedir_SRCS}
)

SET(__homedir_HDRS
        homedir.h
	${__homedir_HDRS}
)

REMOVE_DEFINITIONS(-DQT3_SUPPORT)
REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
SIM_ADD_PLUGIN(__homedir)
ENDIF(BUILD_DROPPED)
