##################
# styles library #
##################

IF(BUILD_DROPPED)
SET(styles_SRCS
        fontconfig.cpp
        styles.cpp
        stylescfg.cpp
)

SET(styles_HDRS
        fontconfig.h
        styles.h
        stylescfg.h
)

SET(styles_UICS
        fontconfigbase.ui
        stylescfgbase.ui
)

REMOVE_DEFINITIONS(-DQT3_SUPPORT)
REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
SIM_ADD_PLUGIN(styles)
ENDIF(BUILD_DROPPED)
