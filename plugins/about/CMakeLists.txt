#################
# about library #
#################
IF(BUILD_DROPPED)

SET(about_SRCS
        about.cpp
        aboutdlg.cpp
)

SET(about_HDRS
        about.h
        aboutdlg.h
)

SET(about_UICS
        aboutdlgbase.ui
)

REMOVE_DEFINITIONS(-DQT3_SUPPORT)
REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
SIM_ADD_PLUGIN(about)
ENDIF(BUILD_DROPPED)
