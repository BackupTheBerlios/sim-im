#################
# ontop library #
#################
IF(BUILD_DROPPED)
SET(ontop_SRCS
        ontop.cpp
        ontopcfg.cpp
)

SET(ontop_HDRS
        ontop.h
        ontopcfg.h
)

SET(ontop_UICS
        ontopcfgbase.ui
)

REMOVE_DEFINITIONS(-DQT3_SUPPORT)
REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
SIM_ADD_PLUGIN(ontop)
ENDIF(BUILD_DROPPED)
